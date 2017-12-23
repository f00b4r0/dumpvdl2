/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "atn-cpdlc.asn1"
 * 	`asn1c -fcompound-names -gen-PER`
 */

#ifndef	_Degrees_H_
#define	_Degrees_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DegreesMagnetic.h"
#include "DegreesTrue.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Degrees_PR {
	Degrees_PR_NOTHING,	/* No components present */
	Degrees_PR_degreesMagnetic,
	Degrees_PR_degreesTrue
} Degrees_PR;

/* Degrees */
typedef struct Degrees {
	Degrees_PR present;
	union Degrees_u {
		DegreesMagnetic_t	 degreesMagnetic;
		DegreesTrue_t	 degreesTrue;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Degrees_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Degrees;

#ifdef __cplusplus
}
#endif

#endif	/* _Degrees_H_ */
#include <asn_internal.h>