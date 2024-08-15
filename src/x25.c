/*
 *  This file is a part of dumpvdl2
 *
 *  Copyright (c) 2017-2023 Tomasz Lemiech <szpajder@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>               // struct timeval
#include <libacars/libacars.h>      // la_type_descriptor, la_proto_node
#include <libacars/vstring.h>       // la_vstring, LA_ISPRINTF()
#include <libacars/dict.h>          // la_dict
#include <libacars/reassembly.h>
#include <libacars/json.h>
#include "config.h"                 // IS_BIG_ENDIAN
#include "dumpvdl2.h"
#include "x25.h"
#include "reassembly.h"             // reasm_contexts
#include "clnp.h"
#include "esis.h"
#include "tlv.h"

/***************************************************************************
 * Packet reassembly types and callbacks
 **************************************************************************/

// Can't use X.25 header as msg_info during reassembly, because
// it does not contain enough addressing information to discriminate
// packes uniquely. We need AVLC addresses to do this.
// This structure is also used as a hash key in reassembly table.
typedef struct {
	uint32_t src_addr, dst_addr;
} x25_avlc_info;

// Allocates X.25 persistent key for a new hash entry.
// As there are no allocations performed for x25_avlc_info structure members,
// it is used as a temporary key allocator as well.
void *x25_key_get(void const *msg) {
	ASSERT(msg != NULL);
	x25_avlc_info const *avlc_info = msg;
	NEW(x25_avlc_info, key);
	key->src_addr = avlc_info->src_addr;
	key->dst_addr = avlc_info->dst_addr;
	debug_print(D_PROTO, "ALLOC KEY %06X %06X\n", key->src_addr, key->dst_addr);
	return (void *)key;
}

void x25_key_destroy(void *ptr) {
	XFREE(ptr);
}

uint32_t x25_key_hash(void const *key) {
	x25_avlc_info const *k = key;
	return k->src_addr * 11 + k->dst_addr * 23;
}

bool x25_key_compare(void const *key1, void const *key2) {
	x25_avlc_info const *k1 = key1;
	x25_avlc_info const *k2 = key2;
	return k1->src_addr == k2->src_addr && k1->dst_addr == k2->dst_addr;
}

static la_reasm_table_funcs x25_reasm_funcs = {
	.get_key = x25_key_get,
	.get_tmp_key = x25_key_get,
	.hash_key = x25_key_hash,
	.compare_keys = x25_key_compare,
	.destroy_key = x25_key_destroy
};

static struct timeval x25_reasm_timeout = {
	.tv_sec = X25_REASM_TIMEOUT_SECONDS,
	.tv_usec = 0
};

void update_statsd_x25_metrics(la_reasm_status reasm_status, uint32_t msg_type) {
	static la_dict const reasm_status_counter_names[] = {
		{ .id = LA_REASM_UNKNOWN, .val = "x25.reasm.unknown" },
		{ .id = LA_REASM_COMPLETE, .val = "x25.reasm.complete" },
		// { .id = LA_REASM_IN_PROGRESS, .val = "x25.reasm.in_progress" },  // report final states only
		{ .id = LA_REASM_SKIPPED, .val = "x25.reasm.skipped" },
		{ .id = LA_REASM_DUPLICATE, .val = "x25.reasm.duplicate" },
		{ .id = LA_REASM_FRAG_OUT_OF_SEQUENCE, .val = "x25.reasm.out_of_seq" },
		{ .id = LA_REASM_ARGS_INVALID, .val = "x25.reasm.invalid_args" },
		{ .id = 0, .val = NULL }
	};
	char const *metric = la_dict_search(reasm_status_counter_names, reasm_status);
	if(metric == NULL) {
		return;
	}
	statsd_increment_per_msgdir(msg_type & MSGFLT_SRC_AIR ? LA_MSG_DIR_AIR2GND : LA_MSG_DIR_GND2AIR, (char *)metric);
	// In case USE_STATSD is disabled
	UNUSED(msg_type);
}

/***************************************************************************
 * Parsers and formatters for X.25 facilities
 **************************************************************************/

/***************************************************************************
 * Max. packet size
 **************************************************************************/

typedef struct {
	uint16_t from_calling_dte, from_called_dte;
} x25_pkt_size_t;

TLV_PARSER(x25_pkt_size_parse) {
	UNUSED(typecode);

	if(len < 2) {
		return NULL;
	}
	if(buf[0] > 0xf || buf[1] > 0xf) {
		return NULL;
	}
	NEW(x25_pkt_size_t, pkt_size);
	pkt_size->from_called_dte = 1 << buf[0];
	pkt_size->from_calling_dte = 1 << buf[1];
	return pkt_size;
}

TLV_FORMATTER(x25_pkt_size_format_text) {
	ASSERT(ctx != NULL);
	ASSERT(ctx->vstr != NULL);
	ASSERT(ctx->indent >= 0);

	x25_pkt_size_t const *pkt_size = data;
	LA_ISPRINTF(ctx->vstr, ctx->indent, "%s:\n", label);
	ctx->indent++;
	LA_ISPRINTF(ctx->vstr, ctx->indent, "From calling DTE: %u bytes\n", pkt_size->from_calling_dte);
	LA_ISPRINTF(ctx->vstr, ctx->indent, "From called  DTE: %u bytes\n", pkt_size->from_called_dte);
	ctx->indent--;
}

TLV_FORMATTER(x25_pkt_size_format_json) {
	ASSERT(ctx != NULL);
	ASSERT(ctx->vstr != NULL);

	x25_pkt_size_t const *pkt_size = data;
	la_json_object_start(ctx->vstr, label);
	la_json_append_int64(ctx->vstr, "from_calling_dte", pkt_size->from_calling_dte);
	la_json_append_int64(ctx->vstr, "from_called_dte", pkt_size->from_called_dte);
	la_json_object_end(ctx->vstr);
}

/***************************************************************************
 * Window size
 **************************************************************************/

typedef struct {
	uint8_t from_calling_dte, from_called_dte;
} x25_win_size_t;

TLV_PARSER(x25_win_size_parse) {
	UNUSED(typecode);

	if(len < 2) {
		return NULL;
	}
	if(buf[0] < 1 || buf[0] > 127 || buf[0] < 1 || buf[1] > 127) {
		return NULL;
	}
	NEW(x25_win_size_t, win_size);
	win_size->from_called_dte = buf[0];
	win_size->from_calling_dte = buf[1];
	return win_size;
}

TLV_FORMATTER(x25_win_size_format_text) {
	ASSERT(ctx != NULL);
	ASSERT(ctx->vstr != NULL);
	ASSERT(ctx->indent >= 0);

	x25_win_size_t const *win_size = data;
	LA_ISPRINTF(ctx->vstr, ctx->indent, "%s:\n", label);
	ctx->indent++;
	LA_ISPRINTF(ctx->vstr, ctx->indent, "From calling DTE: %u packets\n", win_size->from_calling_dte);
	LA_ISPRINTF(ctx->vstr, ctx->indent, "From called  DTE: %u packets\n", win_size->from_called_dte);
	ctx->indent--;
}

TLV_FORMATTER(x25_win_size_format_json) {
	ASSERT(ctx != NULL);
	ASSERT(ctx->vstr != NULL);

	x25_win_size_t const *win_size = data;
	la_json_object_start(ctx->vstr, label);
	la_json_append_int64(ctx->vstr, "from_calling_dte", win_size->from_calling_dte);
	la_json_append_int64(ctx->vstr, "from_called_dte", win_size->from_called_dte);
	la_json_object_end(ctx->vstr);
}

/***************************************************************************
 * Fast select
 **************************************************************************/

typedef struct {
	bool requested, response_restriction;
} x25_fast_select_t;

TLV_PARSER(x25_fast_select_parse) {
	UNUSED(typecode);

	if(len < 1) {
		return NULL;
	}
	NEW(x25_fast_select_t, fs);
	fs->requested = buf[0] & 0x80;
	fs->response_restriction = buf[0] & 0x40;
	return fs;
}

TLV_FORMATTER(x25_fast_select_format_text) {
	ASSERT(ctx != NULL);
	ASSERT(ctx->vstr != NULL);
	ASSERT(ctx->indent >= 0);

	x25_fast_select_t const *fs = data;
	LA_ISPRINTF(ctx->vstr, ctx->indent, "%s: %srequested\n", label,
			fs->requested ? "" : "not ");
}

TLV_FORMATTER(x25_fast_select_format_json) {
	ASSERT(ctx != NULL);
	ASSERT(ctx->vstr != NULL);

	x25_fast_select_t const *fs = data;
	la_json_append_bool(ctx->vstr, label, fs->requested);
}

static la_dict const x25_facilities[] = {
	{
		.id = 0x00,
		.val = &(tlv_type_descriptor_t){
			// This is just a separator - don't parse nor print it
			// .label = "Marker (X.25 facilities follow)",
			.parse = tlv_parser_noop,
			.format_text = NULL,
			.format_json = NULL,
			.destroy = NULL
		}
	},
	{
		.id = 0x01,
		.val = &(tlv_type_descriptor_t){
			.label = "Fast Select",
			.json_key = "fast_select",
			.parse = x25_fast_select_parse,
			.format_text = x25_fast_select_format_text,
			.format_json = x25_fast_select_format_json,
			.destroy = NULL
		}
	},
	{
		.id = 0x08,
		.val = &(tlv_type_descriptor_t){
			.label = "Called line address modified",
			.json_key = "called_line_addr_modified",
			.parse = tlv_octet_string_parse,
			.format_text = tlv_octet_string_format_text,
			.format_json = tlv_octet_string_format_json,
			.destroy = NULL
		}
	},
	{
		.id = 0x42,
		.val = &(tlv_type_descriptor_t){
			.label = "Max. packet size",
			.json_key = "max_pkt_size",
			.parse = x25_pkt_size_parse,
			.format_text = x25_pkt_size_format_text,
			.format_json = x25_pkt_size_format_json,
			.destroy = NULL
		}
	},
	{
		.id = 0x43,
		.val = &(tlv_type_descriptor_t){
			.label = "Window size",
			.json_key = "window_size",
			.parse = x25_win_size_parse,
			.format_text = x25_win_size_format_text,
			.format_json = x25_win_size_format_json,
			.destroy = NULL
		}
	},
	{
		.id = 0xc9,
		.val = &(tlv_type_descriptor_t){
			.label = "Called address extension",
			.json_key = "called_addr_extension",
			.parse = tlv_octet_string_parse,
			.format_text = tlv_octet_string_with_ascii_format_text,
			.format_json = tlv_octet_string_format_json,
			.destroy = NULL
		}
	},
	{
		.id = 0x00,
		.val = NULL
	}
};

static la_dict const x25_comp_algos[] = {
	{ 0x40, "ACA" },
	{ 0x20, "DEFLATE" },
	{ 0x02, "LREF" },
	{ 0x01, "LREF-CAN" },
	{ 0x0,  NULL }
};

/**********************************************************
 * SNDCF Error Report decoder
 **********************************************************/

// Forward declarations
la_type_descriptor const proto_DEF_X25_SNDCF_error_report;
static la_proto_node *parse_x25_user_data(uint8_t *buf, uint32_t len, uint32_t *msg_type,
		reasm_contexts *rtables, struct timeval rx_time, uint32_t src_addr, uint32_t dst_addr);

typedef struct {
	uint8_t error_code;
	uint8_t local_ref;
	bool err;
	bool errored_pdu_present;
} sndcf_err_rpt_t;

static la_proto_node *sndcf_error_report_parse(uint8_t *buf, uint32_t len, uint32_t *msg_type,
		reasm_contexts *rtables, struct timeval rx_time, uint32_t src_addr, uint32_t dst_addr) {
	NEW(sndcf_err_rpt_t, rpt);
	la_proto_node *node = la_proto_node_new();
	node->td = &proto_DEF_X25_SNDCF_error_report;
	node->data = rpt;
	node->next = NULL;

	rpt->err = true;        // fail-safe value
	if(len < 3) {
		debug_print(D_PROTO, "Too short (len %u < min len %u)\n", len, 3);
		goto fail;
	}
	rpt->error_code = buf[1];
	rpt->local_ref = buf[2];
	if(len > 3) {
		// Parse SNDCF Error Report payload, if present.
		// The payload is the PDU which caused the error. It was sent in the other
		// direction, so we have to flip message direction bits for a moment in order
		// to decode this message correctly (this is important if it's CPDLC or CM).
		*msg_type ^= (MSGFLT_SRC_AIR | MSGFLT_SRC_GND);
		node->next = parse_x25_user_data(buf + 3, len - 3, msg_type,
				rtables, rx_time, src_addr, dst_addr);
		*msg_type ^= (MSGFLT_SRC_AIR | MSGFLT_SRC_GND);
		rpt->errored_pdu_present = true;
	} else {
		rpt->errored_pdu_present = false;
	}
	rpt->err = false;
	return node;
fail:
	node->next = unknown_proto_pdu_new(buf, len);
	return node;
}

static char const *sndcf_error_descriptions[] = {
	[0] = "Compressed NPDU with unrecognized Local Reference",
	[1] = "Creation of directory entry outside of sender's permitted range",
	[2] = "Directory entry exists",
	[3] = "Local Reference greater than maximum value accepted",
	[4] = "Data Unit Identifier missing when SP=1",
	[5] = "reserved",
	[6] = "reserved",
	[7] = "Compressed CLNP PDU with unrecognized type",
	[8] = "Local Reference cancellation error"
};
#define SNDCF_ERR_MAX 8

void sndcf_error_report_format_text(la_vstring *vstr, void const *data, int indent) {
	ASSERT(vstr != NULL);
	ASSERT(data);
	ASSERT(indent >= 0);

	sndcf_err_rpt_t const *rpt = data;
	if(rpt->err == true) {
		LA_ISPRINTF(vstr, indent, "%s", "-- Unparseable SNDCF Error Report\n");
		return;
	}
	LA_ISPRINTF(vstr, indent, "%s", "SNDCF Error Report:\n");
	LA_ISPRINTF(vstr, indent+1, "Cause: 0x%02x (%s)\n", rpt->error_code,
			rpt->error_code <= SNDCF_ERR_MAX ? sndcf_error_descriptions[rpt->error_code] : "unknown");
	LA_ISPRINTF(vstr, indent+1, "Local Reference: 0x%02x\n", rpt->local_ref);
	if(rpt->errored_pdu_present) {
		LA_ISPRINTF(vstr, indent, "%s", "Erroneous PDU:\n");
	}
}

void sndcf_error_report_format_json(la_vstring *vstr, void const *data) {
	ASSERT(vstr != NULL);
	ASSERT(data);

	sndcf_err_rpt_t const *rpt = data;
	la_json_append_bool(vstr, "err", rpt->err);
	if(rpt->err == true) {
		return;
	}
	la_json_append_int64(vstr, "cause_code", rpt->error_code);
	if(rpt->error_code <= SNDCF_ERR_MAX) {
		la_json_append_string(vstr, "cause_descr", sndcf_error_descriptions[rpt->error_code]);
	}
	la_json_append_int64(vstr, "local_ref", rpt->local_ref);
	la_json_append_bool(vstr, "erroneous_pdu_present", rpt->errored_pdu_present);
}

la_type_descriptor const proto_DEF_X25_SNDCF_error_report = {
	.format_text = sndcf_error_report_format_text,
	.format_json = sndcf_error_report_format_json,
	.json_key = "sndcf_error_report",
	.destroy = NULL
};

/**********************************************************
 * X.25 decoder
 **********************************************************/

// Forward declaration
la_type_descriptor const proto_DEF_X25_pkt;

static char *fmt_x25_addr(uint8_t const *data, uint8_t len) {
	// len is in nibbles here
	static char const hex[] = "0123456789abcdef";
	char *buf = NULL;
	if(len == 0 || data == NULL) {
		return NULL;
	}
	uint8_t bytelen = (len >> 1) + (len & 1);
	buf = XCALLOC(2 * bytelen + 1, sizeof(char));

	char *ptr = buf;
	int i, j;
	for(i = 0, j = 0; i < bytelen; i++) {
		*ptr++ = hex[((data[i] >> 4) & 0xf)];
		if(++j == len) break;
		*ptr++ = hex[data[i] & 0xf];
		if(++j == len) break;
	}
	return buf;
}

static int parse_x25_address_block(x25_pkt_t *pkt, uint8_t *buf, uint32_t len) {
	if(len == 0) return -1;
	uint8_t calling_len = (*buf & 0xf0) >> 4;   // nibbles
	uint8_t called_len = *buf & 0x0f;           // nibbles
	uint8_t called_len_bytes = (called_len >> 1) + (called_len & 1);        // bytes
	uint8_t calling_len_bytes = (calling_len >> 1) + (calling_len & 1);     // bytes
	uint8_t addr_len = (calling_len + called_len) >> 1;                     // bytes
	addr_len += (calling_len & 1) ^ (called_len & 1);       // add 1 byte if total nibble count is odd
	buf++; len--;
	debug_print(D_PROTO_DETAIL, "calling_len=%u called_len=%u total_len=%u len=%u\n", calling_len, called_len, addr_len, len);
	if(len < addr_len) {
		debug_print(D_PROTO, "Address block truncated (buf len %u < addr len %u)\n", len, addr_len);
		return -1;
	}
	uint8_t *abuf = pkt->called.addr;
	uint8_t *bbuf = pkt->calling.addr;
	memcpy(abuf, buf, called_len_bytes * sizeof(uint8_t));
	uint8_t calling_pos = called_len_bytes - (called_len & 1);
	memcpy(bbuf, buf + calling_pos, (addr_len - calling_pos) * sizeof(uint8_t));
	if(called_len & 1) {
		abuf[called_len_bytes-1] &= 0xf0;
		// shift calling addr one nibble to the left if called addr nibble length is odd
		int i = 0;
		while(i < calling_len >> 1) {
			bbuf[i] = (bbuf[i] << 4) | (bbuf[i+1] >> 4);
			i++;
		}
		if(calling_len & 1)
			bbuf[calling_len_bytes-1] <<= 4;
	}
	pkt->called.len = called_len;
	pkt->calling.len = calling_len;
	pkt->addr_block_present = true;
	return 1 + addr_len;        // return total number of bytes consumed
}

static int parse_x25_callreq_sndcf(x25_pkt_t *pkt, uint8_t *buf, uint32_t len) {
	if(len < 2) return -1;
	if(*buf != X25_SNDCF_ID) {
		debug_print(D_PROTO, "SNDCF identifier not found\n");
		return -1;
	}
	buf++; len--;
	uint8_t sndcf_len = *buf++; len--;
	if(sndcf_len < MIN_X25_SNDCF_LEN || *buf != X25_SNDCF_VERSION) {
		debug_print(D_PROTO, "Unsupported SNDCF field format or version (len=%u ver=%u)\n", sndcf_len, *buf);
		return -1;
	}
	if(len < sndcf_len) {
		debug_print(D_PROTO, "SNDCF field truncated (sndcf_len %u < buf_len %u)\n", sndcf_len, len);
		return -1;
	}
	pkt->compression = buf[3];
	return 2 + sndcf_len;
}

static int parse_x25_facility_field(x25_pkt_t *pkt, uint8_t *buf, uint32_t len) {
	if(len == 0) return -1;
	uint8_t fac_len = *buf;
	buf++; len--;
	if(len < fac_len) {
		debug_print(D_PROTO, "Facility field truncated (buf len %u < fac_len %u)\n", len, fac_len);
		return -1;
	}
	uint8_t i = fac_len;
	// Can't use tlv_parse to parse the whole tag sequence at once, because length field
	// format is non-standard. We have to extract typecode and length and parse the TLV one by one.
	while(i > 0) {
		uint8_t code = *buf;
		uint8_t param_len = (code >> 6) & 3;
		buf++; i--;
		if(param_len < 3) {
			param_len++;
		} else {
			if(i > 0) {
				param_len = *buf++;
				i--;
			} else {
				debug_print(D_PROTO, "Facility field truncated: code=0x%02x param_len=%u, length octet missing\n",
						code, param_len);
				return -1;
			}
		}
		if(i < param_len) {
			debug_print(D_PROTO, "Facility field truncated: code=%02x param_len=%u buf len=%u\n", code, param_len, i);
			return -1;
		}
		pkt->facilities = tlv_single_tag_parse(code, buf, param_len, x25_facilities, pkt->facilities);
		buf += param_len; i -= param_len;
	}
	return 1 + fac_len;
}

static la_proto_node *parse_x25_user_data(uint8_t *buf, uint32_t len, uint32_t *msg_type,
		reasm_contexts *rtables, struct timeval rx_time, uint32_t src_addr, uint32_t dst_addr) {
	if(buf == NULL || len == 0) {
		return NULL;
	}
	uint8_t proto = *buf;
	if(proto == SN_PROTO_CLNP) {
		return clnp_pdu_parse(buf, len, msg_type,
				rtables, rx_time, src_addr, dst_addr);
	} else if(proto == SN_PROTO_ESIS) {
		return esis_pdu_parse(buf, len, msg_type);
	}
	uint8_t pdu_type = proto >> 4;
	if(pdu_type < 0x4 ||
			pdu_type == 0x6 ||
			pdu_type == 0x7 ||
			pdu_type == 0x9 ||
			pdu_type == 0xa) {
		return clnp_compressed_data_pdu_parse(buf, len, msg_type,
				rtables, rx_time, src_addr, dst_addr);
	} else if(proto == 0xe0) {
		return sndcf_error_report_parse(buf, len, msg_type,
				rtables, rx_time, src_addr, dst_addr);
	}
	return unknown_proto_pdu_new(buf, len);
}

la_proto_node *x25_parse(uint8_t *buf, uint32_t len, uint32_t *msg_type,
		reasm_contexts *rtables, struct timeval rx_time, uint32_t src_addr, uint32_t dst_addr) {
	NEW(x25_pkt_t, pkt);
	la_proto_node *node = la_proto_node_new();
	node->td = &proto_DEF_X25_pkt;
	node->data = pkt;
	node->next = NULL;

	pkt->err = true;    // fail-safe value
	uint8_t *ptr = buf;
	uint32_t remaining = len;
	if(remaining < X25_MIN_LEN) {
		debug_print(D_PROTO, "Too short (len %u < min len %u)\n", remaining, X25_MIN_LEN);
		goto fail;
	}

	x25_hdr_t *hdr = (x25_hdr_t *) ptr;
	debug_print(D_PROTO_DETAIL, "gfi=0x%02x group=0x%02x chan=0x%02x type=0x%02x\n", hdr->gfi,
			hdr->chan_group, hdr->chan_num, hdr->type.val);
	if(hdr->gfi != GFI_X25_MOD8) {
		debug_print(D_PROTO, "Unsupported GFI 0x%x\n", hdr->gfi);
		goto fail;
	}

	ptr += sizeof(x25_hdr_t);
	remaining -= sizeof(x25_hdr_t);

	pkt->type = hdr->type.val;
	// Clear out insignificant bits in pkt->type to simplify comparisons later on
	// (hdr->type remains unchanged in case the original value is needed)
	uint8_t pkttype = hdr->type.val;
	if((pkttype & 1) == 0) {
		pkt->type = X25_DATA;
		*msg_type |= MSGFLT_X25_DATA;
	} else {
		pkttype &= 0x1f;
		if(pkttype == X25_RR || pkttype == X25_REJ)
			pkt->type = pkttype;
		*msg_type |= MSGFLT_X25_CONTROL;
	}
	int ret;
	switch(pkt->type) {
		case X25_CALL_REQUEST:
		case X25_CALL_ACCEPTED:
			if((ret = parse_x25_address_block(pkt, ptr, remaining)) < 0) {
				goto fail;
			}
			ptr += ret; remaining -= ret;
			if((ret = parse_x25_facility_field(pkt, ptr, remaining)) < 0) {
				goto fail;
			}
			ptr += ret; remaining -= ret;
			if(pkt->type == X25_CALL_REQUEST) {
				if((ret = parse_x25_callreq_sndcf(pkt, ptr, remaining)) < 0) {
					goto fail;
				}
				ptr += ret; remaining -= ret;
			} else if(pkt->type == X25_CALL_ACCEPTED) {
				if(remaining > 0) {
					pkt->compression = *ptr++;
					remaining--;
				} else {
					debug_print(D_PROTO, "X25_CALL_ACCEPT: no payload\n");
					goto fail;
				}
			}
			// Fast Select is on, so there might be a data PDU in call req or accept
			node->next = parse_x25_user_data(ptr, remaining, msg_type,
					rtables, rx_time, src_addr, dst_addr);
			break;
		case X25_DATA:
			{
				uint8_t *x25_data = ptr;
				uint32_t x25_data_len = remaining;
				pkt->reasm_status = LA_REASM_UNKNOWN;
				bool decode_user_data = true;

				if(rtables->seqbased != NULL) {   // reassembly engine is enabled
					la_reasm_table *x25_rtable = la_reasm_table_lookup(rtables->seqbased, &proto_DEF_X25_pkt);
					if(x25_rtable == NULL) {
						x25_rtable = la_reasm_table_new(rtables->seqbased, &proto_DEF_X25_pkt,
								x25_reasm_funcs, X25_REASM_TABLE_CLEANUP_INTERVAL);
					}
					x25_avlc_info avlc_info = {
						.src_addr = src_addr, .dst_addr = dst_addr
					};
					pkt->reasm_status = la_reasm_fragment_add(x25_rtable,
							&(la_reasm_fragment_info){
							.msg_info = &avlc_info,
							.msg_data = ptr,
							.msg_data_len = remaining,
							.total_pdu_len = 0,     // not used here
							.seq_num = hdr->type.data.sseq,
							.seq_num_first = SEQ_FIRST_NONE,
							.seq_num_wrap = 8,
							.is_final_fragment = hdr->type.data.more ? 0 : 1,
							.rx_time = rx_time,
							.reasm_timeout = x25_reasm_timeout
							});
					int reassembled_len = 0;
					if(pkt->reasm_status == LA_REASM_COMPLETE &&
							(reassembled_len = la_reasm_payload_get(x25_rtable, &avlc_info, &x25_data)) > 0) {
						x25_data_len = reassembled_len;
						// x25_data is a newly allocated buffer; keep the pointer for freeing it later
						pkt->reasm_buf = x25_data;
					} else if((pkt->reasm_status == LA_REASM_IN_PROGRESS ||
								pkt->reasm_status == LA_REASM_DUPLICATE) &&
							Config.decode_fragments == false) {
						decode_user_data = false;
					}
					update_statsd_x25_metrics(pkt->reasm_status, *msg_type);
				}
				node->next = decode_user_data == true ?
					parse_x25_user_data(x25_data, x25_data_len, msg_type,
							rtables, rx_time, src_addr, dst_addr) :
					unknown_proto_pdu_new(x25_data, x25_data_len);
			}
			break;
		case X25_CLEAR_REQUEST:
		case X25_RESET_REQUEST:
		case X25_RESTART_REQUEST:
			if(remaining < 1) {
				goto fail;
			}
			pkt->clr_cause = *ptr++;
			remaining--;
			// When bit 8 is set to 1, the lower bits are those included by the remote DTE in the
			// clearing or restarting cause field of the Clear or Restart Request packet, respectively
			// (X.25, Table 5-7). We don't print the lower bits, so we force the cause code to 0 to have
			// a common value to search for in the dictionary.
			if(pkt->clr_cause & 0x80) {
				pkt->clr_cause = 0;
			}
			if(remaining > 0) {
				pkt->diag_code = *ptr++;
				pkt->diag_code_present = true;
				remaining--;
			}
			break;
		case X25_DIAG:
			if(remaining < 1) {
				goto fail;
			}
			pkt->diag_code = *ptr++;
			pkt->diag_code_present = true;
			remaining--;
			if(remaining > 0) {
				pkt->diag_data.buf = ptr;
				pkt->diag_data.len = remaining;
			}
			break;
		case X25_CLEAR_CONFIRM:
		case X25_RR:
		case X25_REJ:
		case X25_RESET_CONFIRM:
		case X25_RESTART_CONFIRM:
			break;
		default:
			debug_print(D_PROTO, "Unsupported packet identifier 0x%02x\n", pkt->type);
			goto fail;
	}
	pkt->hdr = hdr;
	pkt->err = false;
	return node;
fail:
	node->next = unknown_proto_pdu_new(buf, len);
	return node;
}

static la_dict const x25_clr_causes[] = {
	{ .id = 0x00, .val = "DTE originated" },
	{ .id = 0x01, .val = "Number busy" },
	{ .id = 0x03, .val = "Invalid facility request" },
	{ .id = 0x05, .val = "Network congestion" },
	{ .id = 0x09, .val = "Remote procedure error" },
	{ .id = 0x0d, .val = "Not obtainable" },
	{ .id = 0x13, .val = "Local procedure error" },
	{ .id = 0x15, .val = "ROA out of order" },
	{ .id = 0x19, .val = "Reverse charging acceptance not subscribed" },
	{ .id = 0x21, .val = "Incompatible destination" },
	{ .id = 0x29, .val = "Fast select acceptance not subscribed" },
	{ .id = 0x39, .val = "Ship absent" },
	{ .id = 0x00, .val = NULL }
};

static la_dict const x25_reset_causes[] = {
	{ .id = 0x00, .val = "DTE originated" },
	{ .id = 0x01, .val = "Out of order" },
	{ .id = 0x03, .val = "Remote procedure error" },
	{ .id = 0x05, .val = "Local procedure error" },
	{ .id = 0x07, .val = "Network congestion" },
	{ .id = 0x09, .val = "Remote DTE operational" },
	{ .id = 0x0f, .val = "Network operational" },
	{ .id = 0x11, .val = "Incompatible destination" },
	{ .id = 0x1d, .val = "Network out of order" },
	{ .id = 0x00, .val = NULL }
};

static la_dict const x25_restart_causes[] = {
	{ .id = 0x01, .val = "Local procedure error" },
	{ .id = 0x03, .val = "Network congestion" },
	{ .id = 0x07, .val = "Network operational" },
	{ .id = 0x00, .val = NULL }
};

static la_dict const x25_diag_codes[] = {
	// X.25 Annex E
	{ .id = 0x00, .val = "Cleared by system management" },
	{ .id = 0x01, .val = "Invalid P(S)" },
	{ .id = 0x02, .val = "Invalid P(R)" },
	{ .id = 0x10, .val = "Packet type invalid" },
	{ .id = 0x11, .val = "Packet type invalid for state r1" },
	{ .id = 0x12, .val = "Packet type invalid for state r2" },
	{ .id = 0x13, .val = "Packet type invalid for state r3" },
	{ .id = 0x14, .val = "Packet type invalid for state p1" },
	{ .id = 0x15, .val = "Packet type invalid for state p2" },
	{ .id = 0x16, .val = "Packet type invalid for state p3" },
	{ .id = 0x17, .val = "Packet type invalid for state p4" },
	{ .id = 0x18, .val = "Packet type invalid for state p5" },
	{ .id = 0x19, .val = "Packet type invalid for state p6" },
	{ .id = 0x1a, .val = "Packet type invalid for state p7" },
	{ .id = 0x1b, .val = "Packet type invalid for state d1" },
	{ .id = 0x1c, .val = "Packet type invalid for state d2" },
	{ .id = 0x1d, .val = "Packet type invalid for state d3" },
	{ .id = 0x20, .val = "Packet not allowed" },
	{ .id = 0x21, .val = "Unidentifiable packet" },
	{ .id = 0x22, .val = "Call on one-way logical channel" },
	{ .id = 0x23, .val = "Invalid packet type on a PVC" },
	{ .id = 0x24, .val = "Packet on unassigned logical channel" },
	{ .id = 0x25, .val = "Reject not subscribed to" },
	{ .id = 0x26, .val = "Packet too short" },
	{ .id = 0x27, .val = "Packet too long" },
	{ .id = 0x28, .val = "Invalid general format identifier" },
	{ .id = 0x29, .val = "Restart packet with non-zero reserved bits" },
	{ .id = 0x2a, .val = "Packet type not compatible with facility" },
	{ .id = 0x2b, .val = "Unauthorized interrupt confirmation" },
	{ .id = 0x2c, .val = "Unauthorized interrupt" },
	{ .id = 0x2d, .val = "Unauthorized reject" },
	{ .id = 0x2e, .val = "TOA/NPI address subscription facility not subscribed to" },
	{ .id = 0x30, .val = "Time expired" },
	{ .id = 0x31, .val = "Time expired for incoming call" },
	{ .id = 0x32, .val = "Time expired for clear indication" },
	{ .id = 0x33, .val = "Time expired for reset indication" },
	{ .id = 0x34, .val = "Time expired for restart indication" },
	{ .id = 0x35, .val = "Time expired for call deflection" },
	{ .id = 0x40, .val = "Call setup or call clearing problem" },
	{ .id = 0x41, .val = "Facility code not allowed" },
	{ .id = 0x42, .val = "Facility parameter not allowed" },
	{ .id = 0x43, .val = "Invalid called DTE address" },
	{ .id = 0x44, .val = "Invalid calling DTE address" },
	{ .id = 0x45, .val = "Invalid facility length" },
	{ .id = 0x46, .val = "Incoming call barred" },
	{ .id = 0x47, .val = "No logical channel available" },
	{ .id = 0x48, .val = "Call collision" },
	{ .id = 0x49, .val = "Duplicate facility requested" },
	{ .id = 0x4a, .val = "Non-zero address length" },
	{ .id = 0x4b, .val = "Non-zero facility length" },
	{ .id = 0x4c, .val = "Facility not provided when expected" },
	{ .id = 0x4d, .val = "Invalid ITU-T specified DTE facility" },
	{ .id = 0x4e, .val = "Max number of call redirections or deflections exceeded" },
	{ .id = 0x50, .val = "Miscellaneous" },
	{ .id = 0x51, .val = "Improper cause code from DTE" },
	{ .id = 0x52, .val = "Not aligned octet" },
	{ .id = 0x53, .val = "Inconsistent Q-bit setting" },
	{ .id = 0x54, .val = "NUI problem" },
	{ .id = 0x55, .val = "ICRD problem" },
	{ .id = 0x70, .val = "International problem" },
	{ .id = 0x71, .val = "Remote network problem" },
	{ .id = 0x72, .val = "International protocol problem" },
	{ .id = 0x73, .val = "International link out of order" },
	{ .id = 0x74, .val = "International link busy" },
	{ .id = 0x75, .val = "Transit network facility problem" },
	{ .id = 0x76, .val = "Remote network facility problem" },
	{ .id = 0x77, .val = "International routing problem" },
	{ .id = 0x78, .val = "Temporary routing problem" },
	{ .id = 0x79, .val = "Unknown called DNIC" },
	{ .id = 0x7a, .val = "Maintenance action" },
	// Doc 9705, Table 5.7-3
	{ .id = 0x80, .val = "Version number not supported" },
	{ .id = 0x81, .val = "Invalid length field" },
	{ .id = 0x82, .val = "Call collision resolution" },
	{ .id = 0x83, .val = "Proposed directory size too large" },
	{ .id = 0x84, .val = "LREF cancellation not supported" },
	{ .id = 0x85, .val = "Received DTE refused, received NET refused or invalid NET selector" },
	{ .id = 0x86, .val = "Invalid SNCR field" },
	{ .id = 0x87, .val = "ACA compression not supported" },
	{ .id = 0x88, .val = "LREF compression not supported" },
	{ .id = 0x8f, .val = "Deflate compression not supported" },
	{ .id = 0x90, .val = "Idle timer expired" },
	{ .id = 0x91, .val = "Need to reuse the circuit" },
	{ .id = 0x92, .val = "System local error" },
	{ .id = 0x93, .val = "Invalid SEL field value in received NET" },
	// { .id = 0xd2, .val = "210" },
	// ISO 8208
	{ .id = 0xe1, .val = "OSI network disconnect (transient)" },
	{ .id = 0xe2, .val = "OSI network disconnect (permanent)" },
	{ .id = 0xe3, .val = "OSI network reject - reason unspecified (transient)" },
	{ .id = 0xe4, .val = "OSI network reject - reason unspecified (permanent)" },
	{ .id = 0xe5, .val = "OSI network reject - QoS not available (transient)" },
	{ .id = 0xe6, .val = "OSI network reject - QoS not available (permanent)" },
	{ .id = 0xe7, .val = "OSI network reject - NSAP unreachable (transient)" },
	{ .id = 0xe8, .val = "OSI network reject - NSAP unreachable (permanent)" },
	{ .id = 0xe9, .val = "OSI network reset - no reason given" },
	{ .id = 0xea, .val = "OSI network reset - congestion" },
	{ .id = 0xeb, .val = "OSI network reject - NSAP address unknown (permanent)" },
	// Doc 9705, Table 5.7-3
	{ .id = 0xf0, .val = "System lack of resources" },
	// ISO 8208
	{ .id = 0xf1, .val = "Higher level initiated disconnect (normal)" },
	// Doc 9880, 3.7.4.2.1.6.1.5
	{ .id = 0xf2, .val = "Incompatible information in user data" },
	// ISO 8208
	{ .id = 0xf3, .val = "Higher level initiated disconnect - incompatible data" },
	{ .id = 0xf4, .val = "Higher level initiated reject - no reason given (transient)" },
	{ .id = 0xf5, .val = "Higher level initiated reject - no reason given (permanent)" },
	{ .id = 0xf6, .val = "Higher level initiated reject - QoS not available (transient)" },
	{ .id = 0xf7, .val = "Higher level initiated reject - QoS not available (permanent)" },
	{ .id = 0xf8, .val = "Higher level initiated reject - incompatible data" },
	{ .id = 0xf9, .val = "Unrecognized protocol ID" },
	{ .id = 0xfa, .val = "Higher level initiated reset - user resync" },
	{ .id = 0x00, .val = NULL }
};

static la_dict const x25_pkttype_names[] = {
	{ X25_CALL_REQUEST,     "Call Request" },
	{ X25_CALL_ACCEPTED,    "Call Accepted" },
	{ X25_CLEAR_REQUEST,    "Clear Request" },
	{ X25_CLEAR_CONFIRM,    "Clear Confirm" },
	{ X25_DATA,             "Data" },
	{ X25_RR,               "Receive Ready" },
	{ X25_REJ,              "Receive Reject" },
	{ X25_RESET_REQUEST,    "Reset Request" },
	{ X25_RESET_CONFIRM,    "Reset Confirm" },
	{ X25_RESTART_REQUEST,  "Restart Request" },
	{ X25_RESTART_CONFIRM,  "Restart Confirm" },
	{ X25_DIAG,             "Diagnostics" },
	{ 0,                    NULL }
};

static void x25_format_text(la_vstring *vstr, void const *data, int indent) {
	ASSERT(vstr != NULL);
	ASSERT(data);
	ASSERT(indent >= 0);

	x25_pkt_t const *pkt = data;
	if(pkt->err == true) {
		LA_ISPRINTF(vstr, indent, "%s", "-- Unparseable X.25 packet\n");
		return;
	}
	char const *name = la_dict_search(x25_pkttype_names, pkt->type);
	LA_ISPRINTF(vstr, indent, "X.25 %s: grp: %u chan: %u", name, pkt->hdr->chan_group, pkt->hdr->chan_num);
	if(pkt->addr_block_present) {
		char *calling = fmt_x25_addr(pkt->calling.addr, pkt->calling.len);
		char *called = fmt_x25_addr(pkt->called.addr, pkt->called.len);
		la_vstring_append_sprintf(vstr, " src: %s dst: %s",
				calling ? calling : "none", called ? called : "none");
		XFREE(calling);
		XFREE(called);
	} else if(pkt->type == X25_DATA) {
		la_vstring_append_sprintf(vstr, " sseq: %u rseq: %u more: %u",
				pkt->hdr->type.data.sseq, pkt->hdr->type.data.rseq, pkt->hdr->type.data.more);
	} else if(pkt->type == X25_RR || pkt->type == X25_REJ) {
		la_vstring_append_sprintf(vstr, " rseq: %u", pkt->hdr->type.data.rseq);
	}
	EOL(vstr);
	indent++;

	la_dict const *cause_dict = NULL;
	switch(pkt->type) {
		case X25_CALL_REQUEST:
		case X25_CALL_ACCEPTED:
			LA_ISPRINTF(vstr, indent, "%s", "Facilities:\n");
			tlv_list_format_text(vstr, pkt->facilities, indent+1);
			LA_ISPRINTF(vstr, indent, "%s: ", "Compression support");
			bitfield_format_text(vstr, &pkt->compression, 1, x25_comp_algos);
			EOL(vstr);
			LA_ISPRINTF(vstr, indent, "M/I: %u\n", (pkt->compression & 0x10) != 0);
			break;
		case X25_DATA:
			LA_ISPRINTF(vstr, indent, "X.25 reasm status: %s\n", la_reasm_status_name_get(pkt->reasm_status));
			break;
		case X25_CLEAR_REQUEST:
			cause_dict = x25_clr_causes;
			break;
		case X25_RESET_REQUEST:
			cause_dict = x25_reset_causes;
			break;
		case X25_RESTART_REQUEST:
			cause_dict = x25_restart_causes;
			break;
	}
	if(cause_dict != NULL) {
		char const *clr_cause = la_dict_search(cause_dict, pkt->clr_cause);
		LA_ISPRINTF(vstr, indent, "Cause: 0x%02x (%s)\n", pkt->clr_cause,
				clr_cause ? clr_cause : "unknown");
	}
	if(pkt->diag_code_present) {
		char const *diag_code = la_dict_search(x25_diag_codes, pkt->diag_code);
		LA_ISPRINTF(vstr, indent, "Diagnostic code: 0x%02x (%s)\n", pkt->diag_code,
				diag_code ? diag_code : "unknown");
	}
	if(pkt->type == X25_DIAG && pkt->diag_data.buf != NULL) {
		LA_ISPRINTF(vstr, indent, "%s: ", "Erroneous packet header");
		octet_string_format_text(vstr, &pkt->diag_data, 0);
		EOL(vstr);
	}
}

static void x25_format_json(la_vstring *vstr, void const *data) {
	ASSERT(vstr != NULL);
	ASSERT(data);

	x25_pkt_t const *pkt = data;
	la_json_append_bool(vstr, "err", pkt->err);
	if(pkt->err == true) {
		return;
	}
	la_json_append_int64(vstr, "pkt_type", pkt->type);
	char const *name = la_dict_search(x25_pkttype_names, pkt->type);
	SAFE_JSON_APPEND_STRING(vstr, "pkt_type_name", name);
	la_json_append_int64(vstr, "chan_group", pkt->hdr->chan_group);
	la_json_append_int64(vstr, "chan_num", pkt->hdr->chan_num);

	if(pkt->addr_block_present) {
		char *calling = fmt_x25_addr(pkt->calling.addr, pkt->calling.len);
		SAFE_JSON_APPEND_STRING(vstr, "calling_addr", calling);
		char *called = fmt_x25_addr(pkt->called.addr, pkt->called.len);
		SAFE_JSON_APPEND_STRING(vstr, "called_addr", called);
		XFREE(calling);
		XFREE(called);
	} else if(pkt->type == X25_DATA) {
		la_json_append_int64(vstr, "sseq", pkt->hdr->type.data.sseq);
		la_json_append_int64(vstr, "rseq", pkt->hdr->type.data.rseq);
		la_json_append_bool(vstr, "more", pkt->hdr->type.data.more);
	} else if(pkt->type == X25_RR || pkt->type == X25_REJ) {
		la_json_append_int64(vstr, "rseq", pkt->hdr->type.data.rseq);
	}

	la_dict const *cause_dict = NULL;
	switch(pkt->type) {
		case X25_CALL_REQUEST:
		case X25_CALL_ACCEPTED:
			tlv_list_format_json(vstr, "facilities", pkt->facilities);
			la_json_append_int64(vstr, "compression_options", pkt->compression);
			bitfield_format_json(vstr, &pkt->compression, 1, x25_comp_algos, "compression_algos");
			break;
		case X25_DATA:
			la_json_append_string(vstr, "reasm_status", la_reasm_status_name_get(pkt->reasm_status));
			break;
		case X25_CLEAR_REQUEST:
			cause_dict = x25_clr_causes;
			break;
		case X25_RESET_REQUEST:
			cause_dict = x25_reset_causes;
			break;
		case X25_RESTART_REQUEST:
			cause_dict = x25_restart_causes;
			break;
	}
	if(cause_dict != NULL) {
		la_json_append_int64(vstr, "clear_cause", pkt->clr_cause);
		char const *clr_cause = la_dict_search(cause_dict, pkt->clr_cause);
		SAFE_JSON_APPEND_STRING(vstr, "clear_cause_descr", clr_cause);
	}
	if(pkt->diag_code_present) {
		la_json_append_int64(vstr, "diag_code", pkt->diag_code);
		char const *diag_code = la_dict_search(x25_diag_codes, pkt->diag_code);
		SAFE_JSON_APPEND_STRING(vstr, "diag_code_descr", diag_code);
	}
	if(pkt->type == X25_DIAG && pkt->diag_data.buf != NULL) {
		la_json_append_octet_string(vstr, "erroneous_pkt_hdr", pkt->diag_data.buf, pkt->diag_data.len);
	}
}


static void x25_destroy(void *data) {
	if(data == NULL) {
		return;
	}
	x25_pkt_t *pkt = data;
	if(pkt->facilities != NULL) {
		tlv_list_destroy(pkt->facilities);
		pkt->facilities = NULL;
	}
	XFREE(pkt->reasm_buf);
	XFREE(data);
}

la_type_descriptor const proto_DEF_X25_pkt = {
	.format_text = x25_format_text,
	.format_json = x25_format_json,
	.json_key = "x25",
	.destroy = x25_destroy
};
