/*
 *  dumpvdl2 - a VDL Mode 2 message decoder and protocol analyzer
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

#include <stdbool.h>
#include <math.h>                       // round
#include <time.h>                       // strftime, gmtime, localtime
#include <libacars/libacars.h>          // la_proto_node
#include <libacars/vstring.h>           // la_vstring
#include "fmtr-text.h"
#include "output-common.h"              // fmtr_descriptor_t
#include "dumpvdl2.h"                   // octet_string_t, Config

static bool fmtr_text_supports_data_type(fmtr_input_type_t type) {
	return(type == FMTR_INTYPE_DECODED_FRAME);
}

static la_vstring *format_timestamp(struct timeval tv) {
	struct tm *tmstruct = (Config.utc == true ? gmtime(&tv.tv_sec) : localtime(&tv.tv_sec));

	char tbuf[30], tzbuf[8];
	strftime(tbuf, sizeof(tbuf), "%F %T", tmstruct);
	strftime(tzbuf, sizeof(tzbuf), "%Z", tmstruct);

	la_vstring *vstr = la_vstring_new();
	la_vstring_append_sprintf(vstr, "%s", tbuf);
	if(Config.milliseconds == true) {
		la_vstring_append_sprintf(vstr, ".%03ld", tv.tv_usec / 1000);
	}
	la_vstring_append_sprintf(vstr, " %s", tzbuf);
	return vstr;
}

static octet_string_t *fmtr_text_format_decoded_msg(vdl2_msg_metadata *metadata, la_proto_node *root) {
	ASSERT(metadata != NULL);
	ASSERT(root != NULL);

	la_vstring *timestamp = format_timestamp(metadata->burst_timestamp);
	la_vstring *vstr = la_vstring_new();

	la_vstring_append_sprintf(vstr, "[%s] [%.3f] [%.1f/%.1f dBFS] [%.1f dB] [%.1f ppm]",
			timestamp->str, (float)metadata->freq / 1e+6, metadata->frame_pwr_dbfs, metadata->nf_pwr_dbfs,
			metadata->frame_pwr_dbfs - metadata->nf_pwr_dbfs, metadata->ppm_error);
	la_vstring_destroy(timestamp, true);

	if(Config.extended_header == true) {
		la_vstring_append_sprintf(vstr, " [S:%d] [L:%u] [F:%d] [#%u]",
				metadata->synd_weight, metadata->datalen_octets, metadata->num_fec_corrections, metadata->idx);
	}
	EOL(vstr);

	vstr = la_proto_tree_format_text(vstr, root);
	octet_string_t *ret = octet_string_new(vstr->str, vstr->len);
	la_vstring_destroy(vstr, false);
	return ret;
}

fmtr_descriptor_t fmtr_DEF_text = {
	.name = "text",
	.description = "Human readable text",
	.format_decoded_msg = fmtr_text_format_decoded_msg,
	.format_raw_msg = NULL,
	.supports_data_type = fmtr_text_supports_data_type,
	.output_format = OFMT_TEXT,
};
