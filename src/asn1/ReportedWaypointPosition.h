/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_cpdlc-v1.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#ifndef	_ReportedWaypointPosition_H_
#define	_ReportedWaypointPosition_H_


#include "asn_application.h"

/* Including external dependencies */
#include "Position.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReportedWaypointPosition */
typedef Position_t	 ReportedWaypointPosition_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReportedWaypointPosition;
asn_struct_free_f ReportedWaypointPosition_free;
asn_struct_print_f ReportedWaypointPosition_print;
asn_constr_check_f ReportedWaypointPosition_constraint;
ber_type_decoder_f ReportedWaypointPosition_decode_ber;
der_type_encoder_f ReportedWaypointPosition_encode_der;
xer_type_decoder_f ReportedWaypointPosition_decode_xer;
xer_type_encoder_f ReportedWaypointPosition_encode_xer;
per_type_decoder_f ReportedWaypointPosition_decode_uper;
per_type_encoder_f ReportedWaypointPosition_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _ReportedWaypointPosition_H_ */
#include "asn_internal.h"