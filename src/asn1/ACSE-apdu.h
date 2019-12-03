/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "ACSE-1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_ulcs.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#ifndef	_ACSE_apdu_H_
#define	_ACSE_apdu_H_


#include "asn_application.h"

/* Including external dependencies */
#include "AARQ-apdu.h"
#include "AARE-apdu.h"
#include "RLRQ-apdu.h"
#include "RLRE-apdu.h"
#include "ABRT-apdu.h"
#include "constr_CHOICE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ACSE_apdu_PR {
	ACSE_apdu_PR_NOTHING,	/* No components present */
	ACSE_apdu_PR_aarq,
	ACSE_apdu_PR_aare,
	ACSE_apdu_PR_rlrq,
	ACSE_apdu_PR_rlre,
	ACSE_apdu_PR_abrt,
	/* Extensions may appear below */
	
} ACSE_apdu_PR;

/* ACSE-apdu */
typedef struct ACSE_apdu {
	ACSE_apdu_PR present;
	union ACSE_apdu_u {
		AARQ_apdu_t	 aarq;
		AARE_apdu_t	 aare;
		RLRQ_apdu_t	 rlrq;
		RLRE_apdu_t	 rlre;
		ABRT_apdu_t	 abrt;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ACSE_apdu_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ACSE_apdu;

#ifdef __cplusplus
}
#endif

#endif	/* _ACSE_apdu_H_ */
#include "asn_internal.h"
