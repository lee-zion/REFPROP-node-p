#include <napi.h>
#include "refprop.h"

Refprop::Refprop(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Refprop>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  this->_val = info[0].As<Napi::Number>().DoubleValue();
};

Napi::FunctionReference Refprop::constructor;

void Refprop::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "CRefprop", {});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CRefprop", func);
}

Napi::Object Refprop::NewInstance(Napi::Value arg) {
  Napi::Object obj = constructor.New({arg});
  return obj;
}