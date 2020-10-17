/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_cpdlc-v1.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#include "ReportedWaypointLevel.h"

int
ReportedWaypointLevel_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_Level.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using Level,
 * so here we adjust the DEF accordingly.
 */
static void
ReportedWaypointLevel_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_Level.free_struct;
	td->print_struct   = asn_DEF_Level.print_struct;
	td->check_constraints = asn_DEF_Level.check_constraints;
	td->ber_decoder    = asn_DEF_Level.ber_decoder;
	td->der_encoder    = asn_DEF_Level.der_encoder;
	td->xer_decoder    = asn_DEF_Level.xer_decoder;
	td->xer_encoder    = asn_DEF_Level.xer_encoder;
	td->uper_decoder   = asn_DEF_Level.uper_decoder;
	td->uper_encoder   = asn_DEF_Level.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_Level.per_constraints;
	td->elements       = asn_DEF_Level.elements;
	td->elements_count = asn_DEF_Level.elements_count;
	td->specifics      = asn_DEF_Level.specifics;
}

void
ReportedWaypointLevel_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
ReportedWaypointLevel_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
ReportedWaypointLevel_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
ReportedWaypointLevel_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
ReportedWaypointLevel_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
ReportedWaypointLevel_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

asn_dec_rval_t
ReportedWaypointLevel_decode_uper(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints, void **structure, asn_per_data_t *per_data) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->uper_decoder(opt_codec_ctx, td, constraints, structure, per_data);
}

asn_enc_rval_t
ReportedWaypointLevel_encode_uper(asn_TYPE_descriptor_t *td,
		asn_per_constraints_t *constraints,
		void *structure, asn_per_outp_t *per_out) {
	ReportedWaypointLevel_1_inherit_TYPE_descriptor(td);
	return td->uper_encoder(td, constraints, structure, per_out);
}

static asn_per_constraints_t asn_PER_type_ReportedWaypointLevel_constr_1 GCC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_descriptor_t asn_DEF_ReportedWaypointLevel = {
	"ReportedWaypointLevel",
	"ReportedWaypointLevel",
	ReportedWaypointLevel_free,
	ReportedWaypointLevel_print,
	ReportedWaypointLevel_constraint,
	ReportedWaypointLevel_decode_ber,
	ReportedWaypointLevel_encode_der,
	ReportedWaypointLevel_decode_xer,
	ReportedWaypointLevel_encode_xer,
	ReportedWaypointLevel_decode_uper,
	ReportedWaypointLevel_encode_uper,
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	0, 0,	/* Defined elsewhere */
	0	/* No specifics */
};

