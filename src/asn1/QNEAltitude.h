/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "ADSMessageSetVersion2"
 * 	found in "../../../dumpvdl2.asn1/atn-b2_adsc_v2.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#ifndef	_QNEAltitude_H_
#define	_QNEAltitude_H_


#include "asn_application.h"

/* Including external dependencies */
#include "ADSv2Level.h"

#ifdef __cplusplus
extern "C" {
#endif

/* QNEAltitude */
typedef ADSv2Level_t	 QNEAltitude_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_QNEAltitude;
asn_struct_free_f QNEAltitude_free;
asn_struct_print_f QNEAltitude_print;
asn_constr_check_f QNEAltitude_constraint;
ber_type_decoder_f QNEAltitude_decode_ber;
der_type_encoder_f QNEAltitude_encode_der;
xer_type_decoder_f QNEAltitude_decode_xer;
xer_type_encoder_f QNEAltitude_encode_xer;
per_type_decoder_f QNEAltitude_decode_uper;
per_type_encoder_f QNEAltitude_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _QNEAltitude_H_ */
#include "asn_internal.h"