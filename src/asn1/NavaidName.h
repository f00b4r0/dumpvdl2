/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_cpdlc-v1.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#ifndef	_NavaidName_H_
#define	_NavaidName_H_


#include "asn_application.h"

/* Including external dependencies */
#include "IA5String.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NavaidName */
typedef IA5String_t	 NavaidName_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NavaidName;
asn_struct_free_f NavaidName_free;
asn_struct_print_f NavaidName_print;
asn_constr_check_f NavaidName_constraint;
ber_type_decoder_f NavaidName_decode_ber;
der_type_encoder_f NavaidName_encode_der;
xer_type_decoder_f NavaidName_decode_xer;
xer_type_encoder_f NavaidName_encode_xer;
per_type_decoder_f NavaidName_decode_uper;
per_type_encoder_f NavaidName_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _NavaidName_H_ */
#include "asn_internal.h"
