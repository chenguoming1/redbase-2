#include <sys/times.h>
#include <sys/types.h>
#include <cassert>
#include <unistd.h>
#include <memory>
#include "redbase.h"
#include "ql.h"
#include "sm.h"
#include "ix.h"
#include "rm.h"
#include "parser.h"
#include "ql_internal.h"

using namespace std;

/*
	Implementation of scans
	1. QL_FileScan
	2. QL_IndexScan
*/
QL_FileScan::QL_FileScan(RM_FileHandle &fh, AttrType type, int len, int offset, 
		CompOp cmp, void* value, ClientHint hint, 
		const vector<DataAttrInfo> &attributes) {
	this->fh = &fh;
	this->type = type;
	this->len = len;
	this->offset = offset;
	this->cmp = cmp;
	this->value = value;
	this->hint = hint;
	this->attributes = attributes;
	isOpen = false;
	child = 0;
}

QL_FileScan::~QL_FileScan() {
}

RC QL_FileScan::Open() {
	RC WARN = QL_FILESCAN_WARN, ERR = QL_FILESCAN_ERR;
	if (isOpen) return WARN;
	QL_ErrorForward(fs.OpenScan(*fh, type, len,
            offset, cmp, value, hint));
	isOpen = true;
	return OK_RC;
}

RC QL_FileScan::Next(shared_ptr<char> &rec) {
	RC WARN = QL_FILESCAN_WARN, ERR = QL_FILESCAN_ERR;
	if (!isOpen) return WARN;
	RM_Record record;
	char *temp;
	QL_ErrorForward(fs.GetNextRec(record));
	QL_ErrorForward(record.GetData(temp));
	memcpy(rec.get(), temp, attributes.back().offset + attributes.back().attrLength);
	return OK_RC;
}

RC QL_FileScan::Next(shared_ptr<char> &rec, RID &rid) {
	RC WARN = QL_FILESCAN_WARN, ERR = QL_FILESCAN_ERR;
	if (!isOpen) return WARN;
	RM_Record record;
	char *temp;
	QL_ErrorForward(fs.GetNextRec(record));
	QL_ErrorForward(record.GetData(temp));
	QL_ErrorForward(record.GetRid(rid));
	memcpy(rec.get(), temp, attributes.back().offset + attributes.back().attrLength);
	return OK_RC;
}

RC QL_FileScan::Close() {
	RC WARN = QL_FILESCAN_WARN, ERR = QL_FILESCAN_ERR;
	if (!isOpen) return WARN;
	QL_ErrorForward(fs.CloseScan());
	isOpen = false;
	return OK_RC;
}