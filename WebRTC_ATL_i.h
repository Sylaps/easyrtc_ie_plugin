

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Tue Apr 15 10:35:06 2014
 */
/* Compiler settings for WebRTC_ATL.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0603 
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

#ifndef __WebRTC_ATL_i_h__
#define __WebRTC_ATL_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWebRTCAPI_FWD_DEFINED__
#define __IWebRTCAPI_FWD_DEFINED__
typedef interface IWebRTCAPI IWebRTCAPI;

#endif 	/* __IWebRTCAPI_FWD_DEFINED__ */


#ifndef ___IWebRTCAPIEvents_FWD_DEFINED__
#define ___IWebRTCAPIEvents_FWD_DEFINED__
typedef interface _IWebRTCAPIEvents _IWebRTCAPIEvents;

#endif 	/* ___IWebRTCAPIEvents_FWD_DEFINED__ */


#ifndef __WebRTCAPI_FWD_DEFINED__
#define __WebRTCAPI_FWD_DEFINED__

#ifdef __cplusplus
typedef class WebRTCAPI WebRTCAPI;
#else
typedef struct WebRTCAPI WebRTCAPI;
#endif /* __cplusplus */

#endif 	/* __WebRTCAPI_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IWebRTCAPI_INTERFACE_DEFINED__
#define __IWebRTCAPI_INTERFACE_DEFINED__

/* interface IWebRTCAPI */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IWebRTCAPI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC354B7E-86E1-4952-BA02-ABA7CF069E2A")
    IWebRTCAPI : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE hello( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE run( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE pushToNative( 
            /* [in] */ BSTR cmd,
            /* [in] */ BSTR json) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IWebRTCAPIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWebRTCAPI * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWebRTCAPI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWebRTCAPI * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWebRTCAPI * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWebRTCAPI * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWebRTCAPI * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWebRTCAPI * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *hello )( 
            IWebRTCAPI * This,
            /* [retval][out] */ BSTR *pRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *run )( 
            IWebRTCAPI * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *pushToNative )( 
            IWebRTCAPI * This,
            /* [in] */ BSTR cmd,
            /* [in] */ BSTR json);
        
        END_INTERFACE
    } IWebRTCAPIVtbl;

    interface IWebRTCAPI
    {
        CONST_VTBL struct IWebRTCAPIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWebRTCAPI_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWebRTCAPI_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWebRTCAPI_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWebRTCAPI_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IWebRTCAPI_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IWebRTCAPI_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IWebRTCAPI_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IWebRTCAPI_hello(This,pRet)	\
    ( (This)->lpVtbl -> hello(This,pRet) ) 

#define IWebRTCAPI_run(This)	\
    ( (This)->lpVtbl -> run(This) ) 

#define IWebRTCAPI_pushToNative(This,cmd,json)	\
    ( (This)->lpVtbl -> pushToNative(This,cmd,json) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWebRTCAPI_INTERFACE_DEFINED__ */



#ifndef __WebRTC_ATLLib_LIBRARY_DEFINED__
#define __WebRTC_ATLLib_LIBRARY_DEFINED__

/* library WebRTC_ATLLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WebRTC_ATLLib;

#ifndef ___IWebRTCAPIEvents_DISPINTERFACE_DEFINED__
#define ___IWebRTCAPIEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IWebRTCAPIEvents */
/* [uuid] */ 


EXTERN_C const IID DIID__IWebRTCAPIEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("41D6F559-CC67-4897-A6A7-FAEE37FED5D3")
    _IWebRTCAPIEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IWebRTCAPIEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IWebRTCAPIEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IWebRTCAPIEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IWebRTCAPIEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IWebRTCAPIEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IWebRTCAPIEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IWebRTCAPIEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IWebRTCAPIEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } _IWebRTCAPIEventsVtbl;

    interface _IWebRTCAPIEvents
    {
        CONST_VTBL struct _IWebRTCAPIEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IWebRTCAPIEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _IWebRTCAPIEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _IWebRTCAPIEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _IWebRTCAPIEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _IWebRTCAPIEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _IWebRTCAPIEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _IWebRTCAPIEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IWebRTCAPIEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_WebRTCAPI;

#ifdef __cplusplus

class DECLSPEC_UUID("0E8D29CE-D2D0-459A-8009-3B34EFBC43F0")
WebRTCAPI;
#endif
#endif /* __WebRTC_ATLLib_LIBRARY_DEFINED__ */

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


