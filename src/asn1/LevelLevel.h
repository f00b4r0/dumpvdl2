/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "PMCPDLCMessageSetVersion1"
 * 	found in "../../../dumpvdl2.asn1/atn-b1_cpdlc-v1.asn1"
 * 	`asn1c -fcompound-names -fincludes-quoted -gen-PER`
 */

#ifndef	_LevelLevel_H_
#define	_LevelLevel_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Level;

/* LevelLevel */
typedef struct LevelLevel {
	A_SEQUENCE_OF(struct Level) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LevelLevel_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LevelLevel;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Level.h"

#endif	/* _LevelLevel_H_ */
#include "asn_internal.h"
