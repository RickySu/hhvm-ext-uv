#include "ext.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/url/ext_url.h"
#include "hphp/runtime/server/http-protocol.h"
#include <http_parser.h>

namespace HPHP {

    typedef struct http_parser_ext_s: public http_parser{
        ObjectData *obj;
    } http_parser_ext;

    static int on_message_begin(http_parser *_parser){
//        http_parser_ext *parser = (http_parser_ext *) _parser;
        echo("message begin\n");
        return 0;
    }

    static int on_message_complete(http_parser *_parser){
        http_parser_ext *parser = (http_parser_ext *) _parser;
        parser->obj->o_set("messageComplete", true, s_uvhttpparser);
        return 0;
    }
    
    static int on_headers_complete(http_parser *_parser){
        http_parser_ext *parser = (http_parser_ext *) _parser;
        Array *propHeaderFields = &parser->obj->o_realProp("headerFields", 0, s_uvhttpparser)->asArrRef();
        Array *propHeaderValues = &parser->obj->o_realProp("headerValues", 0, s_uvhttpparser)->asArrRef();
        if(propHeaderFields->size() == propHeaderValues->size()){
            parser->obj->o_set("headers", HHVM_FN(array_combine)(*propHeaderFields, *propHeaderValues), s_uvhttpparser);
        }
        propHeaderFields->clear();
        propHeaderValues->clear();
        parser->obj->o_set("method", StringData::Make(http_method_str((http_method) parser->method)), s_uvhttpparser);
        parser->obj->o_set("headerComplete", true, s_uvhttpparser);
        parser->obj->o_set("keepAlive", !!http_should_keep_alive(parser), s_uvhttpparser);
        return 0;
    }    
    
    static int on_status(http_parser *_parser, const char *buf, size_t len){
//        http_parser_ext *parser = (http_parser_ext *) _parser;
        echo("on status: ");
        echo(StringData::Make(buf, len, CopyString));
        echo("\n");        
        return 0;
    }    
    
    static int on_url(http_parser *_parser, const char *buf, size_t len){
        StaticString key("query");
        Array *parsed_url_array;
        Array result = Array::Create();
        Variant *value;
        http_parser_ext *parser = (http_parser_ext *) _parser;
        Variant parsed_url = HHVM_FN(parse_url)(StringData::Make(buf, len, CopyString), -1);
        if(parsed_url.isArray()){
            parsed_url_array = &parsed_url.asArrRef();
            if(parsed_url_array->exists(key)){
                value = (Variant *)&parsed_url_array->rvalAtRef(key);
                HttpProtocol::DecodeParameters(result, value->asStrRef().data(), value->asStrRef().size());
                vm_call_user_func("var_dump", make_packed_array(result));
                parsed_url_array->set(key, result);
            }
            parser->obj->o_set("url", parsed_url, s_uvhttpparser);
        }
        return 0;
    }
    
    static int on_header_field(http_parser *_parser, const char *buf, size_t len){
        http_parser_ext *parser = (http_parser_ext *) _parser;
        Array *prop = &parser->obj->o_realProp("headerFields", 0, s_uvhttpparser)->asArrRef();
        prop->append(StringData::Make(buf, len, CopyString));
        return 0;
    }
    
    static int on_header_value(http_parser *_parser, const char *buf, size_t len){
        http_parser_ext *parser = (http_parser_ext *) _parser;
        Array *prop = &parser->obj->o_realProp("headerValues", 0, s_uvhttpparser)->asArrRef();
        prop->append(StringData::Make(buf, len, CopyString));        
        return 0;
    }    
    
    static int on_body(http_parser *_parser, const char *buf, size_t len){
//        http_parser_ext *parser = (http_parser_ext *) _parser;
        echo("on body\n");
        return 0;
    }
    
    http_parser_settings parser_settings = {
        on_message_begin,    //on_message_begin
        on_url,    //on_url
        on_status,    //on_status
        on_header_field,    //on_header_field
        on_header_value,    //on_header_value
        on_headers_complete,    //on_headers_complete
        on_body,    //on_body      
        on_message_complete     //on_message_complete
    };
    
    static void HHVM_METHOD(UVHttpParser, __construct, int64_t type) {
        Resource resource(NEWOBJ(InternalResourceData(sizeof(http_parser_ext))));
        SET_RESOURCE(this_, resource, s_uvhttpparser);
        InternalResourceData *http_parser_resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvhttpparser);
        http_parser_ext *parser = (http_parser_ext *) http_parser_resource_data->getInternalResourceData();
        http_parser_init(parser, (http_parser_type) type);
        parser->obj = Object(this_).get();
    }
    
    static int64_t HHVM_METHOD(UVHttpParser, execute, const String &data){
        InternalResourceData *http_parser_resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvhttpparser);
        http_parser_ext *parser = (http_parser_ext *) http_parser_resource_data->getInternalResourceData();
        return http_parser_execute(parser, &parser_settings, data.data(), data.size());
    }
    
    void uvExtension::_initUVHttpParserClass() {
        HHVM_ME(UVHttpParser, __construct);
        HHVM_ME(UVHttpParser, execute);
    }
}