#include "ssl_ext/evidence_ext_structs.hpp"
#include "seats/seats_types.hpp"
#include <cstdio>
#include <cstring>
#include <utility>


void print_string_hex(const unsigned char* s, int len){
    for(int i = 0; i < len; i++)
        printf("%02x", (unsigned int) *s++);
}

int EvidenceType::serialize(const unsigned char** buff){
    // Calculate length
    int len = sizeof(CredentialKind) + sizeof(TypeEncoding);
    int mt_len;
    switch (type_encoding) {
        case TypeEncoding::CONTENT_FORMAT:
            len += sizeof(ContentFormat); 
            break;
        case TypeEncoding::MEDIA_TYPE:
            len += mt_len = strlen(supported_content.media_type) + 1;
            break;
    }

    // Allocate buffer
    const unsigned char *tmp = *buff = (const unsigned char*)new char[len];
    
    *(CredentialKind*)tmp = credential_kind;
    tmp += sizeof(CredentialKind);
    
    *(TypeEncoding*)tmp = type_encoding;
    tmp += sizeof(type_encoding);

    switch (type_encoding) {
        case TypeEncoding::CONTENT_FORMAT:
            *(ContentFormat*)tmp = supported_content.content_format;
            tmp += sizeof(ContentFormat); 
            break;
        case TypeEncoding::MEDIA_TYPE:
            memcpy((void*)tmp, supported_content.media_type, mt_len);
            tmp += mt_len;
            break;
    }
    
    return len;
}

int EvidenceType::deserialize(char* buff){
    char* tmp = buff;
    int mt_len;
    credential_kind = *(CredentialKind*)tmp; 
    tmp += sizeof(CredentialKind);

    type_encoding = *(TypeEncoding*)tmp;
    tmp += sizeof(TypeEncoding);

    switch (type_encoding) {
        case TypeEncoding::CONTENT_FORMAT:
            supported_content.content_format = *(ContentFormat*)tmp;
            tmp += sizeof(ContentFormat); 
            break;
        case TypeEncoding::MEDIA_TYPE:
            mt_len = strlen(tmp) + 1;
            memcpy(supported_content.media_type, (void*)tmp, mt_len);
            tmp += mt_len;
            break;
    }

    return tmp - buff; 
}

int EvidenceRequestClient::serialize(const unsigned char** buff){

    size_t vec_size = supported_evidence_types.size();
    std::vector<std::pair<const unsigned char*, int>> buff_vec;
    const unsigned char* tmp_buff;
    int tmp_len = 0;
    int len = sizeof(size_t) + sizeof(int64_t);
   
    for (EvidenceType& et: supported_evidence_types){
        tmp_len = et.serialize(&tmp_buff); 
        buff_vec.push_back(std::make_pair(tmp_buff, tmp_len)); 
        len+=tmp_len;
    }
    
    tmp_buff = *buff = (const unsigned char*)new char[len]; 
    *(size_t*)tmp_buff = vec_size;
    tmp_buff += sizeof(size_t);
    *(int64_t*)tmp_buff = nonce;
    tmp_buff += sizeof(int64_t);

    for (auto& p: buff_vec){
        memcpy((void*)tmp_buff, p.first, p.second);
        tmp_buff += p.second;
        delete []p.first;
    }
    return len;
}

int EvidenceRequestClient::deserialize(const unsigned char* buff){

    EvidenceType tmp_et;
    const unsigned char *tmp = buff;

    size_t vec_size = *(size_t*)tmp;
    tmp += sizeof(size_t);
    nonce = *(int64_t*)tmp;
    tmp += sizeof(int64_t);

    for (size_t i = 0; i < vec_size; i++){
        tmp += tmp_et.deserialize((char*)tmp); 
        supported_evidence_types.push_back(tmp_et);
    }
    
    return tmp-buff;
}
