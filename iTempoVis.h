

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Tue Jul 23 21:41:11 2013
 */
/* Compiler settings for iTempoVis.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __iTempoVis_h__
#define __iTempoVis_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ITempoVis_FWD_DEFINED__
#define __ITempoVis_FWD_DEFINED__
typedef interface ITempoVis ITempoVis;
#endif 	/* __ITempoVis_FWD_DEFINED__ */


#ifndef __TempoVis_FWD_DEFINED__
#define __TempoVis_FWD_DEFINED__

#ifdef __cplusplus
typedef class TempoVis TempoVis;
#else
typedef struct TempoVis TempoVis;
#endif /* __cplusplus */

#endif 	/* __TempoVis_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ITempoVis_INTERFACE_DEFINED__
#define __ITempoVis_INTERFACE_DEFINED__

/* interface ITempoVis */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITempoVis;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5DFF578-2E89-4438-B900-C56B24C57AFF")
    ITempoVis : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_foregroundColor( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_foregroundColor( 
            /* [in] */ BSTR newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITempoVisVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITempoVis * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITempoVis * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITempoVis * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITempoVis * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITempoVis * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITempoVis * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITempoVis * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_foregroundColor )( 
            ITempoVis * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_foregroundColor )( 
            ITempoVis * This,
            /* [in] */ BSTR newVal);
        
        END_INTERFACE
    } ITempoVisVtbl;

    interface ITempoVis
    {
        CONST_VTBL struct ITempoVisVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITempoVis_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITempoVis_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITempoVis_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITempoVis_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITempoVis_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITempoVis_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITempoVis_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITempoVis_get_foregroundColor(This,pVal)	\
    ( (This)->lpVtbl -> get_foregroundColor(This,pVal) ) 

#define ITempoVis_put_foregroundColor(This,newVal)	\
    ( (This)->lpVtbl -> put_foregroundColor(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITempoVis_INTERFACE_DEFINED__ */



#ifndef __TEMPOVISLib_LIBRARY_DEFINED__
#define __TEMPOVISLib_LIBRARY_DEFINED__

/* library TEMPOVISLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_TEMPOVISLib;

EXTERN_C const CLSID CLSID_TempoVis;

#ifdef __cplusplus

class DECLSPEC_UUID("AC00ACEE-18C3-432a-8F07-6C128033B7EE")
TempoVis;
#endif
#endif /* __TEMPOVISLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


