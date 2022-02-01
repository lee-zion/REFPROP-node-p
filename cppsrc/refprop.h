#ifndef REFPROP_H
#define REFPROP_H

#include "napi.h"

class Refprop : public Napi::ObjectWrap<Refprop> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);
  static Napi::Object NewInstance(Napi::Value arg);
  double Val() const { return _val; }
  Refprop(const Napi::CallbackInfo &info);

 private:
  static Napi::FunctionReference constructor;
  double _val;
};

#endif