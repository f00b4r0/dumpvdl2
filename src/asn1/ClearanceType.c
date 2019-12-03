/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_cpdlc-v1.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#include "ClearanceType.h"

int
ClearanceType_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
ClearanceType_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->uper_decoder   = asn_DEF_NativeEnumerated.uper_decoder;
	td->uper_encoder   = asn_DEF_NativeEnumerated.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_NativeEnumerated.per_constraints;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

void
ClearanceType_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
ClearanceType_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
ClearanceType_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
ClearanceType_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
ClearanceType_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
ClearanceType_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

asn_dec_rval_t
ClearanceType_decode_uper(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints, void **structure, asn_per_data_t *per_data) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->uper_decoder(opt_codec_ctx, td, constraints, structure, per_data);
}

asn_enc_rval_t
ClearanceType_encode_uper(asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints,
		void *structure, asn_per_outp_t *per_out) {
	ClearanceType_1_inherit_TYPE_descriptor(td);
	return td->uper_encoder(td, constraints, structure, per_out);
}

static asn_per_constraints_t asn_PER_type_ClearanceType_constr_1 GCC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  4,  4,  0,  11 }	/* (0..11,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_ClearanceType_value2enum_1[] = {
	{ 0,	13,	"noneSpecified" },
	{ 1,	8,	"approach" },
	{ 2,	9,	"departure" },
	{ 3,	7,	"further" },
	{ 4,	8,	"start-up" },
	{ 5,	8,	"pushback" },
	{ 6,	4,	"taxi" },
	{ 7,	8,	"take-off" },
	{ 8,	7,	"landing" },
	{ 9,	7,	"oceanic" },
	{ 10,	8,	"en-route" },
	{ 11,	10,	"downstream" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_ClearanceType_enum2value_1[] = {
	1,	/* approach(1) */
	2,	/* departure(2) */
	11,	/* downstream(11) */
	10,	/* en-route(10) */
	3,	/* further(3) */
	8,	/* landing(8) */
	0,	/* noneSpecified(0) */
	9,	/* oceanic(9) */
	5,	/* pushback(5) */
	4,	/* start-up(4) */
	7,	/* take-off(7) */
	6	/* taxi(6) */
	/* This list is extensible */
};
static const asn_INTEGER_specifics_t asn_SPC_ClearanceType_specs_1 = {
	asn_MAP_ClearanceType_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_ClearanceType_enum2value_1,	/* N => "tag"; sorted by N */
	12,	/* Number of elements in the maps */
	13,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_ClearanceType_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_ClearanceType = {
	"ClearanceType",
	"ClearanceType",
	ClearanceType_free,
	ClearanceType_print,
	ClearanceType_constraint,
	ClearanceType_decode_ber,
	ClearanceType_encode_der,
	ClearanceType_decode_xer,
	ClearanceType_encode_xer,
	ClearanceType_decode_uper,
	ClearanceType_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ClearanceType_tags_1,
	sizeof(asn_DEF_ClearanceType_tags_1)
		/sizeof(asn_DEF_ClearanceType_tags_1[0]), /* 1 */
	asn_DEF_ClearanceType_tags_1,	/* Same as above */
	sizeof(asn_DEF_ClearanceType_tags_1)
		/sizeof(asn_DEF_ClearanceType_tags_1[0]), /* 1 */
	&asn_PER_type_ClearanceType_constr_1,
	0, 0,	/* Defined elsewhere */
	&asn_SPC_ClearanceType_specs_1	/* Additional specs */
};

