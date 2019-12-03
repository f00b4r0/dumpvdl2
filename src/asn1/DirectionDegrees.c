/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_cpdlc-v1.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#include "DirectionDegrees.h"

static asn_TYPE_member_t asn_MBR_DirectionDegrees_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DirectionDegrees, direction),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_Direction,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"direction"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DirectionDegrees, degrees),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Degrees,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"degrees"
		},
};
static const ber_tlv_tag_t asn_DEF_DirectionDegrees_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DirectionDegrees_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* direction */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* degreesMagnetic */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* degreesTrue */
};
static asn_SEQUENCE_specifics_t asn_SPC_DirectionDegrees_specs_1 = {
	sizeof(struct DirectionDegrees),
	offsetof(struct DirectionDegrees, _asn_ctx),
	asn_MAP_DirectionDegrees_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DirectionDegrees = {
	"DirectionDegrees",
	"DirectionDegrees",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	SEQUENCE_decode_uper,
	SEQUENCE_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DirectionDegrees_tags_1,
	sizeof(asn_DEF_DirectionDegrees_tags_1)
		/sizeof(asn_DEF_DirectionDegrees_tags_1[0]), /* 1 */
	asn_DEF_DirectionDegrees_tags_1,	/* Same as above */
	sizeof(asn_DEF_DirectionDegrees_tags_1)
		/sizeof(asn_DEF_DirectionDegrees_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DirectionDegrees_1,
	2,	/* Elements count */
	&asn_SPC_DirectionDegrees_specs_1	/* Additional specs */
};

