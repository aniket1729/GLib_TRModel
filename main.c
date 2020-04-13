#include "TRTree.c"

Result TR_paramNamesGET(char ***paramNames, int *paramCount);
Result TR_paramNamesFREE(char **paramNames, int paramCount);
Result TR_paramValuesGET(char **paramNamesGET, int *paramCountGET, char ***paramValuesGET);
Result TR_paramValuesFREE(char **paramValuesGET, int paramCountGET);
Result TR_paramValuesSET(char **paramValuesSET, int *paramCountSET);

int main(void)
{
    char** paramNames;
    int    paramCount;
    if( TR_paramNamesGET(&paramNames, &paramCount) == ResultOK )
    {
        DEBUG_LOG("OUTPUT TR_getParamsNames() count= %d,\n", paramCount);
        int i;
        for( i = 0 ; i < paramCount ; i++ )
        {
            DEBUG_LOG("\t%d: %s\n", i+1, paramNames[i]);
        }
    }
    TR_paramNamesFREE(paramNames, paramCount);
    DEBUG_LOG("\n\n");
    
    char* paramNamesGET[] = {
        "Device.INetwork.VoiceServiceNumbers",
/*        "Device.INetwork.LAN.2.Cable.Codec.7.*",
        "Device.INetwork.LAN.2.*",
        "Device.INetwork.LAN.1.LOGS.X_EventLogNumbers",
        "Device.INetwork.LAN.1.Interwork.2.Map.2.Enable",
        "Device.INetwork.LAN.2.Interwork.2.Map.2.Enable",
        "Device.INetwork.LAN.2.Interwork.2.Map.2.enable",
        "Device.INetwork.LAN.1.Interwork.3.Map.2.Enable",
        "Device.INetwork.LAN.1.Interwork.2.Map.3.Enable",
        "Device.INetwork.LAN.0.Interwork.1.Map.1.Enable",
        "Device.INetwork.LAN.1.Interwork.3.Map.1.Enable",
        "Device.INetwork.LAN.3.*",
        "Device.INetwork.WiFi.*",
        "*",
        "Device.INetwork.WiFi.1.Enable",
        "Device.*",*/
    };
    int paramCountGET = sizeof(paramNamesGET)/sizeof(char*);
    DEBUG_LOG("INPUT TR_getParamsValues() count= %d\n", paramCountGET);
    char** paramValuesGET;
    if( TR_paramValuesGET(paramNamesGET, &paramCountGET, &paramValuesGET) == ResultOK )
    {
        DEBUG_LOG("OUTPUT sizeof count= %d,\n", paramCountGET);
paramCountGET = 0;
        int i;
        for( i = 0 ; i < paramCountGET ; i++ )
        {
            DEBUG_LOG("\t%d: %s\n", (i+1), paramValuesGET[i]);
        }
    }
    TR_paramValuesFREE(paramValuesGET, paramCountGET);
    DEBUG_LOG("\n\n");

    
    char* paramValuesSET[] = {
        "Device.INetwork.WiFi.2.Enable, boolean, true",
        "Device.INetwork.LAN.1.Interwork.3.Map.1.Enable, boolean, true",
        "Device.INetwork.INetworkCount, unsignedInt, 2",
        "Device.INetwork.LAN.1.Cable.Codec.1.BitRate, unsignedInt, 1234",
        "Device.INetwork.LAN.1.Cable.Codec.1.Bitrate, unsignedInt, 1234",
        "Device.INetwork.LAN.1.Cable.Codec.1.Codec, string, AMR"
    };
    int paramCountSET = sizeof(paramValuesSET)/sizeof(char*);
    DEBUG_LOG("INPUT TR_paramValuesSET() count= %d\n", paramCountSET);
paramCountSET = 0;
    if( TR_paramValuesSET(paramValuesSET, &paramCountSET) == ResultOK )
    {
        DEBUG_LOG("OUTPUT TR_paramValuesSET() OK\n");
    }
    else
    {
        DEBUG_LOG("OUTPUT TR_paramValuesSET() ERR\n");
    }
    DEBUG_LOG("\n\n");

    
    //RQInfo test = RQInfo(789, NULL, NULL, NULL);
    //DEBUG_LOG("%d %p %p %p\n", test.m_request_type, test.m_param_slist, test.m_request_slist, test.m_val_slist);

    return ResultOK;
}

typedef enum ID_enum { ID_0 = 11, ID_1 = 22, ID_2 = 33, ID_3 = 44, ID_4 = 55, ID_5 = 66, ID_6 = 77, ID_7 = 88} ID_enum;
#define MAX_LINES 3

TRInput g_tr_input[] = {
    { "Device.INetwork.INetworkCount", IndexRangeAutoNoNumeric, ID_0, DataTypeNone},

    { "Device.INetwork.LAN.i", 2, ParamIdDefault, DataTypeNone}, 
    { "Device.INetwork.LAN.i.Cable.Codec.i", 17, ParamIdDefault, DataTypeNone}, 
    { "Device.INetwork.LAN.i.Cable.Codec.i.Codec", IndexRangeAutoNoNumeric, ID_1, DataTypeNone},
    { "Device.INetwork.LAN.i.Cable.Codec.i.BitRate", IndexRangeAutoNoNumeric, ID_2, DataTypeNone},

    { "Device.INetwork.LAN.i.Interwork.i", 2, ParamIdDefault, DataTypeNone},
    { "Device.INetwork.LAN.i.Interwork.i.Map.i", 2, ParamIdDefault, DataTypeNone},
    { "Device.INetwork.LAN.i.Interwork.i.Map.i.Enable", IndexRangeAutoNoNumeric, ID_3, DataTypeNone},


    { "Device.INetwork.LAN.i.Bandwidth.Line.i", MAX_LINES, ParamIdDefault, DataTypeNone}, 
    { "Device.INetwork.LAN.i.Bandwidth.Line.i.Status", IndexRangeAutoNoNumeric, ID_4, DataTypeNone},

    { "Device.INetwork.LAN.i.Bandwidth.Features.SetNumbers", IndexRangeAutoNoNumeric, ID_5, DataTypeNone},


    { "TR.VALUES.APPLY", IndexRangeAutoNoNumeric, ID_6, DataTypeNone}
};

G_Node* g_root = NULL;
#define INIT_ROOT_ONCE    {\
        if( !g_root ) \
        { \
            g_root = TRTree_new(g_tr_input, sizeof(g_tr_input)/sizeof(TRInput)); \
            TRTree_traverse(g_root); \
        } \
    }

Result TR_paramNamesGET(char ***paramNames, int *paramCount)
{
    INIT_ROOT_ONCE;

    G_List* param_names_list = TRTree_getAllParamList(g_root);

    *paramCount = g_slist_length(param_names_list);
    *paramNames = (char**) calloc(*paramCount, sizeof(char*));

    int p_loop = 0;
    G_List* p_itr = param_names_list;
    for ( ; p_itr; p_itr = p_itr->next )
    {
        (*paramNames)[p_loop++] = strdup( (char*)p_itr->data );
    }

    TRTree_freeParamList(param_names_list);
    return ResultOK;
}

Result TR_paramFREE(char **param, int count)
{
    if( count <= 0 )    return ResultOK;

    int p_loop = 0;
    for( ; p_loop < count ; p_loop++ )
    {
        free(param[p_loop]);
    }
    free(param);

   return ResultOK;
}

Result TR_paramNamesFREE(char **paramNames, int paramCount)
{
    return TR_paramFREE(paramNames, paramCount);
}

Result TR_paramValuesGET(char **paramNamesGET, int *paramCountGET, char ***paramValuesGET)
{
    INIT_ROOT_ONCE;

    //TODO:
    return ResultOK;
    RQInfo rqinfo_get;
    TRTree_getAllRequestList(g_root, paramNamesGET, paramCountGET, RequestGETVALS, &rqinfo_get);

    G_List* param_GET_list = rqinfo_get.m_param_slist;
    *paramCountGET = g_slist_length(param_GET_list);
    *paramValuesGET = (char**) calloc(*paramCountGET, sizeof(char*));

    /* Process GET operation */
    int p_loop = 0;
    G_List* p_itr = param_GET_list;
    for ( ; p_itr; p_itr = p_itr->next )
    {
        (*paramValuesGET)[p_loop++] = strdup( (char*)p_itr->data );
    }

    RQInfo_free(&rqinfo_get);
    return ResultOK;
}

Result TR_paramValuesFREE(char **paramValuesGET, int paramCountGET)
{
    return TR_paramFREE(paramValuesGET, paramCountGET);
}

Result TR_paramValuesSET(char **paramValuesSET, int *paramCountSET)
{
    INIT_ROOT_ONCE;

    //TODO:

    return ResultOK;
}