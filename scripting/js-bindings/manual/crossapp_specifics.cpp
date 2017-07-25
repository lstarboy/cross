
#include "crossapp_specifics.hpp"
#include "CrossApp.h"
#include <typeinfo>
#include "js_bindings_config.h"
#include "jsb_crossapp_auto.hpp"
#include "jsapi.h"

using namespace CrossApp;

JSObject* getObjectFromNamespace(JSContext* cx, JS::HandleObject ns, const char *name) {
    JS::RootedValue out(cx);
    bool ok = true;
    if (JS_GetProperty(cx, ns, name, &out) == true) {
        JS::RootedObject obj(cx);
        ok &= JS_ValueToObject(cx, out, &obj);
        JSB_PRECONDITION2(ok, cx, NULL, "Error processing arguments");
    }
    return NULL;
}

jsval anonEvaluate(JSContext *cx, JS::HandleObject thisObj, const char* string) {
    JS::RootedValue out(cx);
    JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
    if (JS_EvaluateScript(cx, thisObj, string, (unsigned int)strlen(string), "(string)", 1, &out) == true) {
        return out.get();
    }
    return JSVAL_VOID;
}



void get_or_create_js_obj(JSContext* cx, JS::HandleObject obj, const std::string &name, JS::MutableHandleObject jsObj)
{
    JS::RootedValue nsval(cx);
    JS_GetProperty(cx, obj, name.c_str(), &nsval);
    if (nsval == JSVAL_VOID) {
        jsObj.set(JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));
        nsval = OBJECT_TO_JSVAL(jsObj);
        JS_SetProperty(cx, obj, name.c_str(), nsval);
    } else {
        jsObj.set(nsval.toObjectOrNull());
    }
}

bool js_forceGC(JSContext *cx, uint32_t argc, jsval *vp) {
    JSRuntime *rt = JS_GetRuntime(cx);
    JS_GC(rt);
    return true;
}

void js_add_object_reference(JS::HandleValue owner, JS::HandleValue target)
{
    if (target.isPrimitive())
    {
        return;
    }
    
    ScriptingCore *engine = ScriptingCore::getInstance();
    JSContext *cx = engine->getGlobalContext();
    JS::RootedObject global(cx, engine->getGlobalObject());
    JS::RootedObject jsbObj(cx);
    get_or_create_js_obj(cx, global, "jsb", &jsbObj);
    JS::RootedValue jsbVal(cx, OBJECT_TO_JSVAL(jsbObj));
    if (jsbVal.isNullOrUndefined())
    {
        return;
    }
    
    JS::RootedValue retval(cx);
    jsval valArr[2];
    valArr[0] = owner;
    valArr[1] = target;
    
    JS::HandleValueArray args = JS::HandleValueArray::fromMarkedLocation(2, valArr);
    engine->executeFunctionWithOwner(jsbVal, "registerNativeRef", args, &retval);
}
void js_remove_object_reference(JS::HandleValue owner, JS::HandleValue target)
{
    if (target.isPrimitive())
    {
        return;
    }
    ScriptingCore *engine = ScriptingCore::getInstance();
    JSContext *cx = engine->getGlobalContext();
    JS::RootedObject ownerObj(cx, owner.toObjectOrNull());
    JS::RootedObject targetObj(cx, target.toObjectOrNull());
    js_proxy_t *pOwner = jsb_get_js_proxy(ownerObj);
    js_proxy_t *pTarget = jsb_get_js_proxy(targetObj);
    if (!pOwner || !pTarget)
    {
        return;
    }
    
    JS::RootedObject global(cx, engine->getGlobalObject());
    JS::RootedObject jsbObj(cx);
    get_or_create_js_obj(cx, global, "jsb", &jsbObj);
    JS::RootedValue jsbVal(cx, OBJECT_TO_JSVAL(jsbObj));
    if (jsbVal.isNullOrUndefined())
    {
        return;
    }
    
    JS::RootedValue retval(cx);
    jsval valArr[2];
    valArr[0] = owner;
    valArr[1] = target;
    
    JS::HandleValueArray args = JS::HandleValueArray::fromMarkedLocation(2, valArr);
    engine->executeFunctionWithOwner(jsbVal, "unregisterNativeRef", args, &retval);
}
void js_add_object_root(JS::HandleValue target)
{
    if (target.isPrimitive())
    {
        return;
    }
    
    ScriptingCore *engine = ScriptingCore::getInstance();
    JSContext *cx = engine->getGlobalContext();
    JS::RootedObject global(cx, engine->getGlobalObject());
    JS::RootedObject jsbObj(cx);
    get_or_create_js_obj(cx, global, "jsb", &jsbObj);
    JS::RootedValue jsbVal(cx, OBJECT_TO_JSVAL(jsbObj));
    if (jsbVal.isNullOrUndefined())
    {
        return;
    }
    
    JS::RootedObject root(cx);
    get_or_create_js_obj(cx, jsbObj, "jsb._root", &root);
    JS::RootedValue valRoot(cx, OBJECT_TO_JSVAL(root));
    
    JS::RootedValue retval(cx);
    jsval valArr[2];
    valArr[0] = valRoot;
    valArr[1] = target;
    
    JS::HandleValueArray args = JS::HandleValueArray::fromMarkedLocation(2, valArr);
    engine->executeFunctionWithOwner(jsbVal, "registerNativeRef", args, &retval);
}
void js_remove_object_root(JS::HandleValue target)
{
    if (target.isPrimitive())
    {
        return;
    }
    ScriptingCore *engine = ScriptingCore::getInstance();
    JSContext *cx = engine->getGlobalContext();
    JS::RootedObject targetObj(cx, target.toObjectOrNull());
    js_proxy_t *pTarget = jsb_get_js_proxy(targetObj);
    if (!pTarget)
    {
        return;
    }
    
    JS::RootedObject global(cx, engine->getGlobalObject());
    JS::RootedObject jsbObj(cx);
    get_or_create_js_obj(cx, global, "jsb", &jsbObj);
    JS::RootedValue jsbVal(cx, OBJECT_TO_JSVAL(jsbObj));
    if (jsbVal.isNullOrUndefined())
    {
        return;
    }
    
    JS::RootedObject root(cx);
    get_or_create_js_obj(cx, jsbObj, "_root", &root);
    JS::RootedValue valRoot(cx, OBJECT_TO_JSVAL(root));
    
    JS::RootedValue retval(cx);
    jsval valArr[2];
    valArr[0] = valRoot;
    valArr[1] = target;
    
    JS::HandleValueArray args = JS::HandleValueArray::fromMarkedLocation(2, valArr);
    engine->executeFunctionWithOwner(jsbVal, "unregisterNativeRef", args, &retval);
}

void register_crossapp_js_core(JSContext* cx, JS::HandleObject global)
{
    JS::RootedObject ccObj(cx);
    JS::RootedObject jsbObj(cx);
    JS::RootedValue tmpVal(cx);
    JS::RootedObject tmpObj(cx);
    get_or_create_js_obj(cx, global, "ca", &ccObj);
    get_or_create_js_obj(cx, global, "jsb", &jsbObj);
    
    tmpObj.set(jsb_CrossApp_CAObject_prototype);
    JS_DefineFunction(cx, global, "garbageCollect", js_forceGC, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(cx, tmpObj, "retain", js_crossapp_retain, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineFunction(cx, tmpObj, "release", js_crossapp_release, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    tmpObj.set(jsb_CrossApp_CAMotionManager_prototype);
    JS_DefineFunction(cx, tmpObj, "startGyroscope", js_crossapp_CAMotionManager_startGyroscope, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    tmpObj.set(jsb_CrossApp_CACustomAnimation_prototype);
    JS_DefineFunction(cx, tmpObj, "schedule", js_crossapp_CACustomAnimation_schedule, 3, JSPROP_READONLY | JSPROP_PERMANENT);
    tmpObj.set(jsb_CrossApp_CADatePickerView_prototype);
    JS_DefineFunction(cx, tmpObj, "onSelectRow", js_crossapp_CADatePickerView_onSelectRow, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    
}

bool js_crossapp_retain(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    CrossApp::CAObject* cobj = (CrossApp::CAObject *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_crossapp_retain : Invalid Native Object");
    
    cobj->retain();
    args.rval().setUndefined();
    return true;
}

bool js_crossapp_release(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    CrossApp::CAObject* cobj = (CrossApp::CAObject *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_crossapp_release : Invalid Native Object");
    
    cobj->release();
    args.rval().setUndefined();
    return true;
}

bool js_crossapp_CAMotionManager_startGyroscope(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    CrossApp::CAMotionManager* cobj = (CrossApp::CAMotionManager *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_crossapp_CAMotionManager_startGyroscope : Invalid Native Object");
    if (argc == 1) {
        std::function<void (const CrossApp::CAMotionManager::Data &)> arg0;
        do {
            if(JS_TypeOfValue(cx, args.get(0)) == JSTYPE_FUNCTION)
            {
                std::shared_ptr<JSFunctionWrapper> func(new JSFunctionWrapper(cx, args.thisv().toObjectOrNull(), args.get(0)));
                auto lambda = [=, &ok](const CrossApp::CAMotionManager::Data & data) -> void {
                    JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
                    jsval largv[1];
                    CAValueMap larg0;
                    larg0["x"] = data.x;
                    larg0["y"] = data.y;
                    larg0["z"] = data.z;
                    larg0["timestamp"] = data.timestamp;
                    largv[0] = cavaluemap_to_jsval(cx, larg0);
                    JS::RootedValue rval(cx);
                    bool succeed = func->invoke(1, &largv[0], &rval);
                    if (!succeed && JS_IsExceptionPending(cx)) {
                        JS_ReportPendingException(cx);
                    }
                };
                arg0 = lambda;
            }
            else
            {
                arg0 = nullptr;
            }
        } while(0)
            ;
        JSB_PRECONDITION2(ok, cx, false, "js_crossapp_CAMotionManager_startGyroscope : Error processing arguments");
        cobj->startGyroscope(arg0);
        args.rval().setUndefined();
        return true;
    }
    
    JS_ReportError(cx, "js_crossapp_CAMotionManager_startGyroscope : wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}

bool js_crossapp_CADatePickerView_onSelectRow(JSContext *cx, uint32_t argc, jsval *vp)
{
    
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    CrossApp::CADatePickerView* cobj = (CrossApp::CADatePickerView *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_crossapp_CADatePickerView_onSelectRow : Invalid Native Object");
    if (argc == 1) {
        std::function<void (const tm &)> arg0;
        do {
            if(JS_TypeOfValue(cx, args.get(0)) == JSTYPE_FUNCTION)
            {
                std::shared_ptr<JSFunctionWrapper> func(new JSFunctionWrapper(cx, args.thisv().toObjectOrNull(), args.get(0)));
                auto lambda = [=, &ok](const tm & var) -> void {
                    JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
                    jsval largv[1];
                    CAValueMap larg0;
                    larg0["tm_sec"] = var.tm_sec;
                    larg0["tm_min"] = var.tm_min;
                    larg0["tm_hour"] = var.tm_hour;
                    larg0["tm_mday"] = var.tm_mday;
                    larg0["tm_mon"] = var.tm_mon;
                    larg0["tm_year"] = var.tm_year;
                    larg0["tm_wday"] = var.tm_wday;
                    larg0["tm_yday"] = var.tm_yday;
                    larg0["tm_isdst"] = var.tm_isdst;
                    larg0["tm_gmtoff"] = (int)var.tm_gmtoff;
                    larg0["tm_zone"] = (const char*)var.tm_zone;
                    largv[0] = cavaluemap_to_jsval(cx, larg0);
                    JS::RootedValue rval(cx);
                    bool succeed = func->invoke(1, &largv[0], &rval);
                    if (!succeed && JS_IsExceptionPending(cx)) {
                        JS_ReportPendingException(cx);
                    }
                };
                arg0 = lambda;
            }
            else
            {
                arg0 = nullptr;
            }
        } while(0)
            ;
        JSB_PRECONDITION2(ok, cx, false, "js_crossapp_CADatePickerView_onSelectRow : Error processing arguments");
        cobj->onSelectRow(arg0);
        args.rval().setUndefined();
        return true;
    }
    
    JS_ReportError(cx, "js_crossapp_CADatePickerView_onSelectRow : wrong number of arguments: %d, was expecting %d", argc, 1);
    return false;
}
bool js_crossapp_CACustomAnimation_schedule(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
    js_proxy_t *proxy = jsb_get_js_proxy(obj);
    CrossApp::CADatePickerView* cobj = (CrossApp::CADatePickerView *)(proxy ? proxy->ptr : NULL);
    JSB_PRECONDITION2( cobj, cx, false, "js_crossapp_CACustomAnimation_schedule : Invalid Native Object");
    
    do {
        if (argc == 3) {
            std::function<void (const CrossApp::CACustomAnimation::Model &)> arg0;
            do {
                if(JS_TypeOfValue(cx, args.get(0)) == JSTYPE_FUNCTION)
                {
                    std::shared_ptr<JSFunctionWrapper> func(new JSFunctionWrapper(cx, args.thisv().toObjectOrNull(), args.get(0)));
                    auto lambda = [=, &ok](const CrossApp::CACustomAnimation::Model & var) -> void {
                        JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
                        jsval largv[1];
                        CAValueMap larg0;
                        larg0["dt"]     = CAValue(var.dt);
                        larg0["now"]    = CAValue(var.now);
                        larg0["total"]  = CAValue(var.total);
                        larg0["end"]    = CAValue(var.end);
                        largv[0] = cavaluemap_to_jsval(cx, larg0);
                        JS::RootedValue rval(cx);
                        bool succeed = func->invoke(1, &largv[0], &rval);
                        if (!succeed && JS_IsExceptionPending(cx)) {
                            JS_ReportPendingException(cx);
                        }
                    };
                    arg0 = lambda;
                }
                else
                {
                    arg0 = nullptr;
                }
            } while(0)
                ;
            if (!ok) { ok = true; break; }
            std::string arg1;
            ok &= jsval_to_std_string(cx, args.get(1), &arg1);
            if (!ok) { ok = true; break; }
            double arg2 = 0;
            ok &= JS::ToNumber( cx, args.get(2), &arg2) && !isnan(arg2);
            if (!ok) { ok = true; break; }
            JSB_PRECONDITION2(ok, cx, false, "js_crossapp_CACustomAnimation_schedule : Error processing arguments");
            CrossApp::CACustomAnimation::schedule(arg0, arg1, arg2);
            args.rval().setUndefined();
            return true;
        }
    } while (0);
    
    do {
        if (argc == 4) {
            std::function<void (const CrossApp::CACustomAnimation::Model &)> arg0;
            do {
                if(JS_TypeOfValue(cx, args.get(0)) == JSTYPE_FUNCTION)
                {
                    std::shared_ptr<JSFunctionWrapper> func(new JSFunctionWrapper(cx, args.thisv().toObjectOrNull(), args.get(0)));
                    auto lambda = [=, &ok](const CrossApp::CACustomAnimation::Model & var) -> void {
                            JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
                            jsval largv[1];
                            CAValueMap larg0;
                            larg0["dt"]     = CAValue(var.dt);
                            larg0["now"]    = CAValue(var.now);
                            larg0["total"]  = CAValue(var.total);
                            larg0["end"]    = CAValue(var.end);
                            largv[0] = cavaluemap_to_jsval(cx, larg0);
                            JS::RootedValue rval(cx);
                            bool succeed = func->invoke(1, &largv[0], &rval);
                            if (!succeed && JS_IsExceptionPending(cx)) {
                                JS_ReportPendingException(cx);
                            }
                        };
                    arg0 = lambda;
                }
                else
                {
                    arg0 = nullptr;
                }
            } while(0)
                ;
            if (!ok) { ok = true; break; }
            std::string arg1;
            ok &= jsval_to_std_string(cx, args.get(1), &arg1);
            if (!ok) { ok = true; break; }
            double arg2 = 0;
            ok &= JS::ToNumber( cx, args.get(2), &arg2) && !isnan(arg2);
            if (!ok) { ok = true; break; }
            double arg3 = 0;
            ok &= JS::ToNumber( cx, args.get(3), &arg3) && !isnan(arg3);
            if (!ok) { ok = true; break; }
            JSB_PRECONDITION2(ok, cx, false, "js_crossapp_CACustomAnimation_schedule : Error processing arguments");
            CrossApp::CACustomAnimation::schedule(arg0, arg1, arg2, arg3);
            args.rval().setUndefined();
            return true;
        }
    } while (0);
    
    do {
        if (argc == 5) {
            std::function<void (const CrossApp::CACustomAnimation::Model &)> arg0;
            do {
                if(JS_TypeOfValue(cx, args.get(0)) == JSTYPE_FUNCTION)
                {
                    std::shared_ptr<JSFunctionWrapper> func(new JSFunctionWrapper(cx, args.thisv().toObjectOrNull(), args.get(0)));
                    auto lambda = [=, &ok](const CrossApp::CACustomAnimation::Model & var) -> void {
                        JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
                        jsval largv[1];
                        CAValueMap larg0;
                        larg0["dt"]     = CAValue(var.dt);
                        larg0["now"]    = CAValue(var.now);
                        larg0["total"]  = CAValue(var.total);
                        larg0["end"]    = CAValue(var.end);
                        largv[0] = cavaluemap_to_jsval(cx, larg0);
                        JS::RootedValue rval(cx);
                        bool succeed = func->invoke(1, &largv[0], &rval);
                        if (!succeed && JS_IsExceptionPending(cx)) {
                            JS_ReportPendingException(cx);
                        }
                    };
                    arg0 = lambda;
                }
                else
                {
                    arg0 = nullptr;
                }
            } while(0)
                ;
            if (!ok) { ok = true; break; }
            std::string arg1;
            ok &= jsval_to_std_string(cx, args.get(1), &arg1);
            if (!ok) { ok = true; break; }
            double arg2 = 0;
            ok &= JS::ToNumber( cx, args.get(2), &arg2) && !isnan(arg2);
            if (!ok) { ok = true; break; }
            double arg3 = 0;
            ok &= JS::ToNumber( cx, args.get(3), &arg3) && !isnan(arg3);
            if (!ok) { ok = true; break; }
            double arg4 = 0;
            ok &= JS::ToNumber( cx, args.get(4), &arg4) && !isnan(arg4);
            if (!ok) { ok = true; break; }
            JSB_PRECONDITION2(ok, cx, false, "js_crossapp_CACustomAnimation_schedule : Error processing arguments");
            CrossApp::CACustomAnimation::schedule(arg0, arg1, arg2, arg3, arg4);
            args.rval().setUndefined();
            return true;
        }
    } while (0);
    JS_ReportError(cx, "js_crossapp_CACustomAnimation_schedule : wrong number of arguments");
    return false;
}
