#ifndef TRTREE_H
#define TRTREE_H

#include <glib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG_LOG(msg, ...)    printf(msg, ##__VA_ARGS__);

typedef gchar* G_ChrPtr;
typedef gboolean G_Bool;
typedef gpointer G_Ptr;
typedef guint G_UInt;
typedef GSList G_List;
typedef GNode G_Node;

typedef enum Result { ResultOK, ResultERR } Result;
typedef enum IndexRange { IndexRangeInvalid = 0, IndexRangeAutoNoNumeric } IndexRange;    /* Other range values can be greater than equal to 1 */
typedef enum ParamId { ParamIdInvalid = -2, ParamIdDefault } ParamId;    /* Other IDs will start from 0 */ 
typedef enum DataType { DataTypeNone = 0 } DataType;    /* Other datatype will have value more than 0 */

typedef struct TRInput TRInput;
struct TRInput { 
    const char      *m_object; 
    const IndexRange m_leaf_range;    /* TODO: either range value or retrieval id */
    const ParamId    m_leaf_id;
    const DataType   m_leaf_data_type;
};

typedef struct TRGen TRGen;
struct TRGen
{
    char*      m_name;

    IndexRange m_index_range;
    ParamId    m_id;
    DataType   m_data_type;
};
TRGen* TRGen_newParam(const char* param_name);
TRGen* TRGen_makeParam(const char* param_name, const int index_range, const int id, const int data_type);
Result TRGen_freeParam(const TRGen* tr_param);
Result TRGen_printParam(const TRGen* tr_param);
G_Bool TRGen_freeParamNode(G_Node *node, gpointer data);
G_Bool TRGen_printParamNode(G_Node *node, gpointer data);

typedef struct TRRequest TRRequest;
struct TRRequest
{
    ParamId m_id;
    G_List* m_indices;    /* Set of IndexRange */
};

typedef enum RequestType { RequestNAMES = 0, RequestGETVALS, RequestSETVALS } RequestType;
typedef struct RQInfo RQInfo;
struct RQInfo
{
    RequestType m_request_type;

    G_List*    m_param_slist;    /* RequestNAMES, RequestGETVALS, RequestSETVALS */      /* Set of G_ChrPtr */
    G_List*    m_request_slist;  /* RequestGETVALS, RequestSETVALS */                    /* Set of TRRequest */
    G_List*    m_val_slist;      /* RequestSETVALS */                                    /* Set of G_ChrPtr */
};
G_Ptr RQInfo_isNonEmpty(const RQInfo *const rq_info);
Result RQInfo_concat(RQInfo *const rq_info1, const RQInfo *const rq_info2);
Result RQInfo_setNULL(RQInfo *const rq_info);
Result RQInfo_next(RQInfo *const rq_info);
G_ChrPtr RQInfo_get_curParam(const RQInfo *const rq_info);
ParamId RQInfo_get_curId(const RQInfo *const rq_info);
Result RQInfo_set_curId(const RQInfo *const rq_info, ParamId param_id);
G_List* RQInfo_get_curIndices(const RQInfo *const rq_info);
Result RQInfo_setNULL_curIndices(RQInfo *const rq_info);
Result RQInfo_append(RQInfo *const rq_info, const G_ChrPtr const param_name, const TRRequest *const tr_request, const G_ChrPtr const val_str);
Result RQInfo_free(RQInfo *const rq_info);



#define TRInput_SEPARATOR_CHAR '.'
#define TRInput_INDEX_FORMAT   "i"
    #define IS_TRInput_PARAM_NUMERIC(param_name)        ( 0 == strcmp(param_name, TRInput_INDEX_FORMAT) )
    #define IS_TRInput_PARAM_NON_NUMERIC(param_name)    ( 0 != strcmp(param_name, TRInput_INDEX_FORMAT) )

#define TRRequest_SEPARATOR_CHAR ','
#define TRRequest_WILDCARD     "*"
    #define IS_TRRequest_PARAM_WILDCARD(tr_param)       ( 0 != strcmp(tr_param, TRRequest_WILDCARD) )
#define TRRequest_WILDCARD_END ".*"
    #define IS_TRReqeust_OBJECT_WILDCARD(tr_object)     ( strncmp( (tr_object) +strlen((tr_object))-2, TRRequest_WILDCARD_END, 2) == 0 )    /* WILDCARD at END, as "<>.*" (Compare last 2 chars) */
    #define IS_TRRequest_PARAM_NUMERIC(tr_param)        ( strlen(tr_param) > 0 && isdigit(tr_param[0]) )

//#define TRInput_INDEX_STR_LEN    10
#define PARAM_NULL NULL
#define VALUE_NULL NULL

Result TRGen_getSelfInfo(const G_Node *const node, RQInfo* rqinfo_self)
{
    TRGen* node_data = (TRGen*)(node->data);

    if( IS_TRInput_PARAM_NON_NUMERIC(node_data->m_name) )
    {
        G_ChrPtr self_param_name = g_strdup(node_data->m_name);
            
        TRRequest* self_request = g_new(TRRequest, 1);
        self_request->m_id = node_data->m_id;
        self_request->m_indices = NULL;

        RQInfo_append(rqinfo_self, self_param_name, self_request, VALUE_NULL);
    }
    else
    {
        int p_loop = 0;
        for( ; p_loop < node_data->m_index_range ; p_loop++ )
        {                    
            //G_ChrPtr sub_param_name = g_strdup_printf("%*d", -TRInput_INDEX_STR_LEN, p_loop);
            G_ChrPtr sub_param_name = g_strdup_printf("%d", (p_loop+1));

            TRRequest* sub_request = g_new(TRRequest, 1);
            sub_request->m_id = node_data->m_id;
            sub_request->m_indices = g_slist_append(NULL, GINT_TO_POINTER(p_loop+1) );

            RQInfo_append(rqinfo_self, sub_param_name, sub_request, VALUE_NULL);
        }
    }
    return ResultOK;
}

#define IS_PARAMID_LEAF(param_id)        ( (param_id) >  ParamIdDefault )    /* ( (param_id) != ParamIdInvalid && (param_id) != ParamIdDefault ) */
#define IS_PARAMID_NON_LEAF(param_id)    ( (param_id) <= ParamIdDefault )    /* ( (param_id) == ParamIdInvalid || (param_id) == ParamIdDefault ) */

#define RQ_INFO_EMPTY_SLIST(request_type)    {request_type, NULL, NULL, NULL}

void TRGen_getRQInfo(G_Node *node, gpointer out_param)
{
    //if( !node )    return;

    TRGen* node_data = (TRGen*)(node->data);
    //if( !node_data )    return;
   
    RQInfo* rqinfo_out = out_param;
    
    if( rqinfo_out->m_request_type != RequestNAMES && rqinfo_out->m_request_type != RequestGETVALS )
        return;
    if( node_data->m_index_range <= IndexRangeInvalid )
        return;

    /* Self List */
    RQInfo rqinfo_self = RQ_INFO_EMPTY_SLIST(rqinfo_out->m_request_type);
    TRGen_getSelfInfo(node, &rqinfo_self);

    /* Child List */
    RQInfo rqinfo_all_child = RQ_INFO_EMPTY_SLIST(rqinfo_out->m_request_type);
    if( RQInfo_isNonEmpty(&rqinfo_self) && IS_PARAMID_NON_LEAF(node_data->m_id) && g_node_n_children(node) > 0 )
    {
        RQInfo child_info = RQ_INFO_EMPTY_SLIST(rqinfo_out->m_request_type);
        g_node_children_foreach(node, G_TRAVERSE_ALL, (GNodeForeachFunc)TRGen_getRQInfo, &child_info);

        RQInfo_concat(&rqinfo_all_child, &child_info);
        RQInfo_setNULL(&child_info);
    }

    /* Append to rqinfo_out */
    if( RQInfo_isNonEmpty(&rqinfo_self) && RQInfo_isNonEmpty(&rqinfo_all_child) )
    {
        RQInfo s_itr = rqinfo_self;
        for ( ; RQInfo_isNonEmpty(&s_itr) ; RQInfo_next(&s_itr) )
        {
            RQInfo c_itr = rqinfo_all_child;
            for ( ; RQInfo_isNonEmpty(&c_itr) ; RQInfo_next(&c_itr) )
            {
                /* rqinfo_out = rqinfo_self + TRInput_SEPARATOR_STR + rqinfo_all_child */
                G_ChrPtr join_param_name = g_strdup_printf("%s%c%s", RQInfo_get_curParam(&s_itr), TRInput_SEPARATOR_CHAR, RQInfo_get_curParam(&c_itr));

                TRRequest* join_request = g_new(TRRequest, 1);
                join_request->m_id = RQInfo_get_curId(&c_itr);
                join_request->m_indices = g_slist_copy( RQInfo_get_curIndices(&s_itr) );
                join_request->m_indices = g_slist_concat( join_request->m_indices, RQInfo_get_curIndices(&c_itr) );
                
                RQInfo_append(rqinfo_out, join_param_name, join_request, VALUE_NULL);

                RQInfo_setNULL_curIndices(&c_itr);
            }
            RQInfo_setNULL_curIndices(&s_itr);
        }

        RQInfo_free(&rqinfo_self);
        RQInfo_free(&rqinfo_all_child);
    }
    else
    {
        RQInfo_concat(rqinfo_out, &rqinfo_self);    /* concat() is ok even if rqinfo_self has NULL list. So skipping additional NULL check */
        RQInfo_setNULL(&rqinfo_self);
    }
}


#include <string.h>
#include <stdlib.h>

#define TRTree_ROOT "<root>"

#define GET_SUBSTRING_1st(string, separator_char, sub_string_1st, seperator_pos)    { \
        seperator_pos = strchr(string, separator_char); \
        if( seperator_pos ) \
        { \
            sub_string_1st = strndup(string, seperator_pos - string); \
        } \
        else \
        { \
            sub_string_1st = strdup(string); \
        } \
    }

typedef enum InsertFlag { InsertYesIfMissing, InsertNo } InsertFlag;
const G_Node* TRTree_childSerachInsert(G_Node *const node, const char *const tr_param, const InsertFlag insert_flag)
{
    const G_Node *node_child = node->children;
    for( ; node_child ; node_child = node_child->next )
    {
        if( 0 == strcmp( tr_param, ((TRGen*)node_child->data)->m_name) )
            break;
    }

    if( node_child == NULL && insert_flag == InsertYesIfMissing )
    {
        node_child = g_node_new( TRGen_newParam(tr_param) );
        g_node_append(node, (G_Node*)node_child);
    }

    return node_child;
}

TRGen* const TRTree_insertObject(G_Node* node, const char *const tr_object)
{
    char* tr_param = NULL;
    char* tr_param_sep_child = NULL;
    GET_SUBSTRING_1st(tr_object, (int)TRInput_SEPARATOR_CHAR, tr_param, tr_param_sep_child);


    const G_Node* nodeChild = TRTree_childSerachInsert(node, tr_param, InsertYesIfMissing);
    free(tr_param);


    if( tr_param_sep_child )    /* No end of TR OBJECT, then insert remaining part */
        return TRTree_insertObject( (G_Node*) nodeChild, tr_param_sep_child+1);
    else                        /* Reached end of TR OBJECT */
        return (TRGen *const)(nodeChild->data);
}

G_Node* TRTree_new(const TRInput trObjectInfo[], const int tr_objects_count)
{
    G_Node* root = g_node_new( TRGen_newParam(TRTree_ROOT) );

    int i;
    for( i = 0 ; i < tr_objects_count ; i++ )
    {
        TRGen *const addedParam = TRTree_insertObject(root, trObjectInfo[i].m_object);
        if( addedParam )
        {
            addedParam->m_index_range = trObjectInfo[i].m_leaf_range;
            addedParam->m_id          = trObjectInfo[i].m_leaf_id;
            addedParam->m_data_type   = trObjectInfo[i].m_leaf_data_type;

            //DEBUG_LOG("Added ");
            //TRGen_printParam(addedParam);
        }
    }

    return root;
}

Result TRTree_free(const G_Node* root)
{
    g_node_traverse((G_Node*)root, G_POST_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)TRGen_freeParamNode, NULL);
    g_node_destroy((G_Node*)root);
    return ResultOK;
}

Result TRTree_traverse(const G_Node* root)
{
    DEBUG_LOG("\n");
    g_node_traverse((G_Node*)root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)TRGen_printParamNode, NULL);
    DEBUG_LOG("\n");
    return ResultOK;
}


G_List* TRTree_getAllParamList(const G_Node *const root)
{
    RQInfo rqinfo_get = RQ_INFO_EMPTY_SLIST(RequestNAMES);
 
    g_node_children_foreach((G_Node*)root, G_TRAVERSE_ALL, (GNodeForeachFunc)TRGen_getRQInfo, &rqinfo_get);

    return rqinfo_get.m_param_slist;    /* Local variable 'rqinfo_get' destroyed. But returning pointer in it */    //TODO: free
}
Result TRTree_freeParamList(G_List* const param_slist)
{
    g_slist_foreach(param_slist, (GFunc)g_free, NULL);
    g_slist_free(param_slist);
    return ResultOK;
}

#define IS_IN_RANGE(xLow, x, xHigh)    ( (xLow) <= (x) && (x) <= (xHigh) )
#define INVALID_NODE_NULL NULL
Result TRTree_locateObjectAndSub(const G_Node* const node, const char *const tr_object, RQInfo *const rqinfo_getset)
{
    TRGen* node_data = (TRGen*)(node->data);

    char* tr_param = NULL;
    char* tr_param_sep_child = NULL;
    GET_SUBSTRING_1st(tr_object, (int)TRInput_SEPARATOR_CHAR, tr_param, tr_param_sep_child);


    if( IS_TRRequest_PARAM_WILDCARD(tr_param) )
    {
        if( tr_param_sep_child == NULL )
        {
            free(tr_param);
            
            RQInfo rqinfo_getsub = RQ_INFO_EMPTY_SLIST(rqinfo_getset->m_request_type);        
            g_node_children_foreach((G_Node*)node, G_TRAVERSE_ALL, (GNodeForeachFunc)TRGen_getRQInfo, &rqinfo_getsub);

            //TODO: append rqinfo_getset indices to rqinfo_getsub
            //TODO: change rqinfo_getset to return rqinfo_getsub

            return ResultOK;
        } /* else tr_object not-finished at WILDCARD */
    }
    else if( IS_TRRequest_PARAM_NUMERIC(tr_param) )
    {
        G_Node* param_node_found = TRTree_childSerachInsert((G_Node*)node, TRInput_INDEX_FORMAT, InsertNo);
        if( param_node_found )
        {
            IndexRange tr_param_index_val = atoi(tr_param);
            if( IS_IN_RANGE(IndexRangeAutoNoNumeric, tr_param_index_val, node_data->m_index_range) )
            {
                G_List* rqinfo_indices = RQInfo_get_curIndices(rqinfo_getset);
                rqinfo_indices = g_slist_append(rqinfo_indices, GINT_TO_POINTER(tr_param_index_val) );

                if( tr_param_sep_child != NULL )
                {
                    free(tr_param);

                    rqinfo_getset->m_param_slist = g_slist_append(rqinfo_getset->m_param_slist, &node_data->m_id);
                    return TRTree_locateObjectAndSub(param_node_found, tr_param_sep_child+1, rqinfo_getset);
                } /* else tr_object finished at NUMERIC */
            } /* else NUMERIC index value out of range */
        } /* else node doesn't have NUMERIC child */
    }
    else    /* PARAM TEXT */
    {
        G_Node* param_node_found = TRTree_childSerachInsert((G_Node*)node, tr_param, InsertNo);
        if( param_node_found )
        {
            TRGen* node_found_data = (TRGen*)(param_node_found->data);
            if( IS_TRInput_PARAM_NON_NUMERIC(node_found_data->m_name) )
            {
                if( tr_param_sep_child != NULL )
                {
                    free(tr_param);

                    RQInfo_set_curId(rqinfo_getset, node_found_data->m_id);
                    return ResultOK;
                }
                else
                {
                    free(tr_param);

                    return TRTree_locateObjectAndSub(param_node_found, tr_param_sep_child+1, rqinfo_getset);
                }
            } /* else PARAM TEXT matching with NUMERIC child */
        } /* else PARAM not found in Tree */
    } /* else PARAM neither a WILDCARD or NUMERIC or TEXT */

    free(tr_param);
    return ResultERR;
}

Result TRTree_extractRequest(const char *const request_str, G_ChrPtr* tr_object, G_ChrPtr* tr_object_pos_valuetype, G_ChrPtr* tr_object_pos_value)
{
    char* tr_object_sep_valuetype = NULL;
    GET_SUBSTRING_1st(request_str, (int)TRRequest_SEPARATOR_CHAR, *tr_object, tr_object_sep_valuetype);

    if( tr_object_sep_valuetype )
    {
        *tr_object_pos_valuetype = (tr_object_sep_valuetype+1);
        if( *tr_object_pos_valuetype )
        {
            char *tr_object_valuetype= NULL;
            char *tr_object_sep_value= NULL;
            GET_SUBSTRING_1st(*tr_object_pos_valuetype, (int)TRRequest_SEPARATOR_CHAR, tr_object_valuetype, tr_object_sep_value);
            free(tr_object_valuetype);

            *tr_object_pos_value = (tr_object_sep_value+1);
        }
    }

    return ResultOK;
}
Result TRTree_getAllRequestList(const G_Node *const root, char **param_values, int *param_values_count, RequestType request_type, RQInfo* rqinfo_getset)
{
    if( request_type != RequestGETVALS || request_type != RequestSETVALS )
        return ResultERR;

    //CHECK
    int p_loop = 0;
    for( ; p_loop < *param_values_count ; p_loop++ )
    {
        G_ChrPtr tr_object = PARAM_NULL;
        G_ChrPtr tr_object_pos_valuetype;
        G_ChrPtr tr_object_pos_value = VALUE_NULL;
        TRTree_extractRequest(param_values[p_loop], &tr_object, &tr_object_pos_valuetype, &tr_object_pos_value);

        if( ( request_type != RequestGETVALS && request_type != RequestSETVALS )
        || ( request_type == RequestGETVALS &&  tr_object_pos_valuetype )                  /* INVALID: GET request with VALUE TYPE */
        || ( request_type == RequestSETVALS && !tr_object_pos_value )                      /* INVALID: SET request without VALUE */
        || ( request_type == RequestSETVALS && IS_TRReqeust_OBJECT_WILDCARD(tr_object) ) ) /* INVALID: SET request with WILDCARD end */
        {
            TRRequest* request_getset_invalid = g_new(TRRequest, 1);
            request_getset_invalid->m_id = ParamIdInvalid;
            request_getset_invalid->m_indices = NULL;

            RQInfo_append(rqinfo_getset, tr_object, request_getset_invalid, tr_object_pos_value);
        }
        else
        {
            RQInfo rqinfo_getset_valid = RQ_INFO_EMPTY_SLIST(request_type);
            if( ResultERR == TRTree_locateObjectAndSub(root, tr_object, &rqinfo_getset_valid) )
            {
                TRRequest* request_getset_fail = g_new(TRRequest, 1);
                request_getset_fail->m_id = ParamIdInvalid;
                request_getset_fail->m_indices = NULL;

                RQInfo_append(rqinfo_getset, tr_object, request_getset_fail, tr_object_pos_value);

                RQInfo_free(&rqinfo_getset_valid);
            }
            else
            {
                //TODO: append tr_object to RQInfo m_param_slist
                //TODO: append tr_object_pos_value to RQInfo m_val_slist (only for single RequestSETVALS)
                RQInfo_concat(rqinfo_getset, &rqinfo_getset_valid);
            }
        }
    }

    return ResultOK;
}



TRGen* TRGen_newParam(const char* param_name)
{
    TRGen* newParam = g_new(TRGen, 1);

    newParam->m_name        = strdup(param_name);
    newParam->m_index_range = IndexRangeAutoNoNumeric;
    newParam->m_id          = ParamIdDefault;
    newParam->m_data_type   = DataTypeNone;

    return newParam;
}
TRGen* TRGen_makeParam(const char* param_name, const int index_range, const int id, const int data_type)
{
    TRGen* newParam = g_new(TRGen, 1);

    newParam->m_name        = strdup(param_name);
    newParam->m_index_range = index_range;
    newParam->m_id          = id;
    newParam->m_data_type   = data_type;

    return newParam;
}
Result TRGen_freeParam(const TRGen* tr_param)
{
    if( tr_param )  
    {
        free(tr_param->m_name);
    }
    g_free((gpointer)tr_param);
    return ResultOK;
}

Result TRGen_printParam(const TRGen* tr_param)
{
    DEBUG_LOG("node= %s", tr_param->m_name); 
    switch( tr_param->m_index_range )
    {
        case IndexRangeAutoNoNumeric: DEBUG_LOG(" range= Auto #%d", tr_param->m_index_range);    break;
        default: DEBUG_LOG(" range= %d", tr_param->m_index_range);    break;
    }
    switch( tr_param->m_id )
    {
        case ParamIdInvalid: DEBUG_LOG(" id= Invalid #%d ", tr_param->m_id);    break;
        case ParamIdDefault: DEBUG_LOG(" id= Default #%d ", tr_param->m_id);    break;
        default: DEBUG_LOG(" id= %d ", tr_param->m_id);    break;
    }
    switch( tr_param->m_data_type )
    {
        case DataTypeNone: DEBUG_LOG(" datatype= None #%d ", DataTypeNone);    break;
        default: DEBUG_LOG(" datatype= %d ", tr_param->m_data_type);    break;
    }
    DEBUG_LOG("\n");
    return ResultOK;
}

G_Bool TRGen_freeParamNode(G_Node *node, gpointer data)
{
    TRGen_freeParam(node->data);
    return FALSE;
}
G_Bool TRGen_printParamNode(G_Node *node, gpointer data)
{
    G_UInt depth = g_node_depth(node);
    G_UInt i = 0;
    for( ; i < depth ; i++ )
    {
        if( i < (depth-1) )
        {
            DEBUG_LOG("   ");
        }
        else
        {
            DEBUG_LOG("|--");
        }
    }
    if(node->data)
        TRGen_printParam(node->data);

    return FALSE;
}


G_Ptr RQInfo_isNonEmpty(const RQInfo *const rq_info)
{return (rq_info->m_param_slist);
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :   return (rq_info->m_param_slist);
        case RequestGETVALS : return (rq_info->m_request_slist);  //OR m_param_slist
        case RequestSETVALS : return (rq_info->m_val_slist);      //OR m_request_slist
        default :             return NULL;
    }
}

Result RQInfo_concat(RQInfo *const rq_info1, const RQInfo *const rq_info2)
{
    if( rq_info1->m_request_type != rq_info1->m_request_type )
        return ResultERR;

    switch( rq_info1->m_request_type )
    {
        case RequestNAMES :
        case RequestGETVALS :
        case RequestSETVALS :
            rq_info1->m_param_slist = g_slist_concat(rq_info1->m_param_slist, rq_info2->m_param_slist);
            rq_info1->m_request_slist = g_slist_concat(rq_info1->m_request_slist, rq_info2->m_request_slist);
            rq_info1->m_val_slist = g_slist_concat(rq_info1->m_val_slist, rq_info2->m_val_slist);
            return ResultOK;

        default :    return ResultERR;
    }
}
Result RQInfo_setNULL(RQInfo *const rq_info)
{
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
        case RequestGETVALS :
        case RequestSETVALS :
            rq_info->m_param_slist = NULL;
            rq_info->m_request_slist = NULL;
            rq_info->m_val_slist = NULL;
            return ResultOK;

        default :    return ResultERR;
    }
}

Result RQInfo_next(RQInfo *const rq_info)
{
            rq_info->m_param_slist = rq_info->m_param_slist->next;
            rq_info->m_request_slist = rq_info->m_request_slist->next;
            return ResultOK;
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
            rq_info->m_param_slist = rq_info->m_param_slist->next;
            return ResultOK;

        case RequestGETVALS :
            rq_info->m_param_slist = rq_info->m_param_slist->next;
            rq_info->m_request_slist = rq_info->m_request_slist->next;
            return ResultOK;

        case RequestSETVALS :
            rq_info->m_request_slist = rq_info->m_request_slist->next;
            rq_info->m_val_slist = rq_info->m_val_slist->next;
            return ResultOK;

        default :    return ResultERR;
    }
}
G_ChrPtr RQInfo_get_curParam(const RQInfo *const rq_info)
{
            return (G_ChrPtr)rq_info->m_param_slist->data;
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
            return (G_ChrPtr)rq_info->m_param_slist->data;

        case RequestGETVALS :
        case RequestSETVALS :
        default :
            return "";
    }
}
ParamId RQInfo_get_curId(const RQInfo *const rq_info)
{
            return ((TRRequest*)rq_info->m_request_slist->data)->m_id;
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
        case RequestGETVALS :
            return ((TRRequest*)rq_info->m_request_slist->data)->m_id;

        case RequestSETVALS :
        default :
            return ParamIdInvalid;
    }
}

Result RQInfo_set_curId(const RQInfo *const rq_info, ParamId param_id)
{
    ((TRRequest*)rq_info->m_request_slist->data)->m_id = param_id; 
    return ResultOK;
}
G_List* RQInfo_get_curIndices(const RQInfo *const rq_info)
{
            return ((TRRequest*)rq_info->m_request_slist->data)->m_indices;
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
        case RequestGETVALS :
            return ((TRRequest*)rq_info->m_request_slist->data)->m_indices;

        case RequestSETVALS :
        default :
            return NULL;
    }
}
Result RQInfo_setNULL_curIndices(RQInfo *const rq_info)
{
            ((TRRequest*)rq_info->m_request_slist->data)->m_indices = NULL;
            return ResultOK;
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
        case RequestGETVALS :
            ((TRRequest*)rq_info->m_request_slist->data)->m_indices = NULL;
            return ResultOK;

        case RequestSETVALS :
        default :
            return ResultERR;
    }
}

Result RQInfo_append(RQInfo *const rq_info, const G_ChrPtr const param_name, const TRRequest *const tr_request, const G_ChrPtr const val_str)
{
            rq_info->m_param_slist = g_slist_append(rq_info->m_param_slist, param_name);
            rq_info->m_request_slist = g_slist_append(rq_info->m_request_slist, (TRRequest*)tr_request);
            return ResultOK;
    switch( rq_info->m_request_type )
    {
        case RequestNAMES :
            rq_info->m_param_slist = g_slist_append(rq_info->m_param_slist, param_name);
            return ResultOK;

        case RequestGETVALS :
            rq_info->m_param_slist = g_slist_append(rq_info->m_param_slist, param_name);
            rq_info->m_request_slist = g_slist_append(rq_info->m_request_slist, (TRRequest*)tr_request);
            return ResultOK;

        case RequestSETVALS :
            rq_info->m_request_slist = g_slist_append(rq_info->m_request_slist, (TRRequest*)tr_request);
            rq_info->m_val_slist = g_slist_append(rq_info->m_val_slist, val_str);
            return ResultOK;

        default :
            return ResultERR;
    }
}

Result RQInfo_free(RQInfo *const rq_info)
{
    g_slist_foreach(rq_info->m_param_slist, (GFunc)g_free, NULL);
    g_slist_free(rq_info->m_param_slist);
    g_slist_foreach(rq_info->m_request_slist, (GFunc)g_free, NULL);
    g_slist_free(rq_info->m_request_slist);
    g_slist_foreach(rq_info->m_val_slist, (GFunc)g_free, NULL);
    g_slist_free(rq_info->m_val_slist);
    return ResultOK;
}

#endif    //TRTREE_H