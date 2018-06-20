//
// Created by sebastian on 22.03.18.
//

#include "TypeDB.h"
#include <form.h>
#include <iostream>

namespace typeart {

std::string TypeDB::builtinNames[] = {"char", "uchar", "short", "ushort", "int",    "uint",
                                      "long", "ulong", "float", "double", "unknown"};

TypeInfo TypeDB::InvalidType = TypeInfo{BUILTIN, UNKNOWN};

std::string TypeDB::UnknownStructName{"UnknownStruct"};

TypeDB::TypeDB() = default;

void TypeDB::clear() {
  structInfoList.clear();
  id2Idx.clear();
  // reverseTypeMap.clear();
}

bool TypeDB::isBuiltinType(int id) const {
  return id < N_BUILTIN_TYPES;
}

bool TypeDB::isStructType(int id) const {
  return id2Idx.find(id) != id2Idx.end();
}

bool TypeDB::isValid(int id) const {
  if (isBuiltinType(id)) {
    return true;
  }
  return id2Idx.find(id) != id2Idx.end();
}

void TypeDB::registerStruct(StructTypeInfo structType) {
  if (isValid(structType.id)) {
    std::cerr << "Invalid type ID for struct " << structType.name << std::endl;
    if (isBuiltinType(structType.id)) {
      std::cerr << "Type ID is reserved for builtin types" << std::endl;
    } else {
      std::cerr << "Conflicting struct is " << getStructInfo(structType.id)->name << std::endl;
    }
    // TODO: Error handling
    return;
  }
  structInfoList.push_back(structType);
  id2Idx.insert({structType.id, structInfoList.size() - 1});
  // reverseTypeMap.insert({id, typeName});
}

const std::string& TypeDB::getTypeName(int id) const {
  if (isBuiltinType(id)) {
    return builtinNames[id];
  }
  if (isStructType(id)) {
    const auto* structInfo = getStructInfo(id);
    if (structInfo) {
      return structInfo->name;
    }
  }
  return UnknownStructName;
}

size_t TypeDB::getBuiltinTypeSize(int id) const {
  switch (id) {
    case C_CHAR:
    case C_UCHAR:
      return sizeof(char);
    case C_SHORT:
    case C_USHORT:
      return sizeof(short);
    case C_INT:
    case C_UINT:
      return sizeof(int);
    case C_LONG:
    case C_ULONG:
      return sizeof(long);
    case C_FLOAT:
      return sizeof(float);
    case C_DOUBLE:
      return sizeof(double);
    default:
      return 0;
  }
}

const StructTypeInfo* TypeDB::getStructInfo(int id) const {
  auto it = id2Idx.find(id);
  if (it != id2Idx.end()) {
    return &structInfoList[it->second];
  }
  return nullptr;
}

TypeInfo TypeDB::getTypeInfo(int id) const {
  if (isBuiltinType(id)) {
    return TypeInfo{BUILTIN, id};
  }
  if (isStructType(id)) {
    return TypeInfo{STRUCT, id};
  }
  return InvalidType;
}

const std::vector<StructTypeInfo>& TypeDB::getStructList() const {
  return structInfoList;
}

}  // namespace typeart
