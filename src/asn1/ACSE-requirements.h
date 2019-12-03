/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "ACSE-1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_ulcs.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#ifndef	_ACSE_requirements_H_
#define	_ACSE_requirements_H_


#include "asn_application.h"

/* Including external dependencies */
#include "BIT_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ACSE_requirements {
	ACSE_requirements_authentication	= 0,
	ACSE_requirements_application_context_negotiation	= 1
} e_ACSE_requirements;

/* ACSE-requirements */
typedef BIT_STRING_t	 ACSE_requirements_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ACSE_requirements;
asn_struct_free_f ACSE_requirements_free;
asn_struct_print_f ACSE_requirements_print;
asn_constr_check_f ACSE_requirements_constraint;
ber_type_decoder_f ACSE_requirements_decode_ber;
der_type_encoder_f ACSE_requirements_encode_der;
xer_type_decoder_f ACSE_requirements_decode_xer;
xer_type_encoder_f ACSE_requirements_encode_xer;
per_type_decoder_f ACSE_requirements_decode_uper;
per_type_encoder_f ACSE_requirements_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _ACSE_requirements_H_ */
#include "asn_internal.h"
