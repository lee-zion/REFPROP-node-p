#include "refprop.h"
#include "napi.h"
#define REFPROP_IMPLEMENTATION
#include "REFPROP_lib.h"
#undef REFPROP_IMPLEMENTATION

using namespace Napi;

Napi::Object AllinOne(const Napi::CallbackInfo &info) {
  // argument definition
  /*
  0 : Path        1 : lib Name    2 : isMixture   3 : fldName
  4 : hIn         5 : firstArg    6 : secondArg   7 : hOut
  8 : isSuperGet  9 : hOutLength  10: isPropType  11 : begin_table
  12: end_table   13: step_table

  isMixture ? SetFluids : SetMixture

  isSuperGet ? hOut : "D,VIS,KV,TCX,CP,CV,PRANDTL,STN,H,PCEST"
  (10 Properties, hOutLength = 10)

  isPropType ? Single Result : Array of Result

  step_table: Maximum 250 points
  */
  Env env = info.Env();

  std::string absPath = info[0].As<String>().Utf8Value();
  std::string dllName = info[1].As<String>().Utf8Value();
  std::string fldName = info[3].As<String>().Utf8Value();
  std::string hIn = info[4].As<String>().Utf8Value();
  bool isMixture = info[2].As<Boolean>();
  bool isSuperGet = info[8].As<Boolean>();
  bool isPropType = info[9].As<Boolean>();
  double firstArg = info[5].As<Number>().DoubleValue();
  double secondArg = info[6].As<Number>().DoubleValue();

  double begin_table = 0.0, step_table = 0.1;
  int hOutLength = 0, num_table = 1;

  if (isPropType == false) {
    begin_table = info[10].As<Number>().DoubleValue();
    step_table = info[11].As<Number>().DoubleValue();
    num_table = info[12].As<Number>().Int32Value();
    if (num_table > 250) {
      num_table = 250;
    } else if (num_table < 0) {
      num_table = 1;
    }
  }

  std::string hOut;
  if (isSuperGet == false) {
    // Get Den, Vis, DyVis, kTh, Cp, Cv, Pr, STN, h, Pcrit, Tcrit
    hOut = "T,P,D,VIS,KV,TCX,CP,CV,PRANDTL,STN,H,PCEST,TCEST";
  } else {
    hOut = info[7].As<String>().Utf8Value();
  }

  std::string propStr[20];
  int i_prop = 0, i_comma = 0, i_comma_prev = 0;
  do {
    i_comma = hOut.find(",", i_comma_prev);
    std::string temp = hOut.substr(i_comma_prev, i_comma - i_comma_prev);
    propStr[i_prop++] = temp;
    hOutLength++;
    if (i_comma == -1) {
      break;
    }
    i_comma_prev = i_comma + 1;
  } while (1);

  std::string err;

  bool loaded_REFPROP = load_REFPROP(err, absPath, dllName);
  if (!loaded_REFPROP) {
    Napi::Value retValue = Napi::String::New(env, err.c_str());
    return retValue.ToObject();
  }
  SETPATHdll(const_cast<char *>(absPath.c_str()), 400);

  char hEnum[255] = "MASS BASE SI", herr[255];
  int iFlag = 0, iEnum = 0, ierr = 0, iMass = 1;
  GETENUMdll(iFlag, hEnum, iEnum, ierr, herr, 255, 255);
  if (ierr > 0) {
    Napi::Value retValue = Napi::String::New(env, herr);
    return retValue.ToObject();
  }

  double comp[20] = {1.0};
  if (isMixture == true) {
    int ierr = 0;
    SETMIXTUREdll(const_cast<char *>(fldName.c_str()), comp, ierr, 10000);
    if (ierr > 0) {
      char herrmsg[255];
      ERRMSGdll(ierr, herrmsg, 255);
      return Napi::String::From(env, herrmsg).ToObject();
    }
  } else {
    int ierr = 0;
    SETFLUIDSdll(const_cast<char *>(fldName.c_str()), ierr, 10000);
    if (ierr > 0) {
      char herrmsg[255];
      ERRMSGdll(ierr, herrmsg, 255);
      return Napi::String::From(env, herrmsg).ToObject();
    }
  }

  double Output[200], liquidPhase[20], vaporPhase[20], x3[20], q;
  char hUnits[255];
  int iUcode = 0;

  Napi::Array resArr = Napi::Array::New(env);

  if (isPropType == true) {
    Napi::Object resVal = Napi::Object::New(env);
    REFPROPdll(const_cast<char *>(fldName.c_str()),
               const_cast<char *>(hIn.c_str()),
               const_cast<char *>(hOut.c_str()), iEnum, iMass, iFlag, firstArg,
               secondArg, comp, Output, hUnits, iUcode, liquidPhase, vaporPhase,
               x3, q, ierr, herr, 10000, 255, 255, 255, 255);
    if (ierr > 0) {
      Napi::Value retValue = Napi::Value::From(env, herr);
      return retValue.ToObject();
    }
    for (int i = 0; i < hOutLength; i++) {
      resVal.Set(propStr[i], Output[i]);
    }
    resArr.Set(1, resVal);
  } else {
    // REFPROPdll manipulates the composition, so assign the copied one
    double cpycomp[20];

    for (int i_tbl = 0; i_tbl < num_table; i_tbl++) {
      Napi::Object resVal = Napi::Object::New(env);
      secondArg = begin_table + step_table * i_tbl;

      // Related to BUG in REFPROP:
      // https://github.com/usnistgov/REFPROP-wrappers/issues/229
      if (isMixture == true) {
        for (int i_cp = 0; i_cp < 20; i_cp++) {
          cpycomp[i_cp] = comp[i_cp];
        }
      }
      // if (isMixture == true)
      // {
      //     printf("BF RP: %f %f %f\n", comp[0], comp[1], comp[2]);
      // }
      REFPROPdll(
          const_cast<char *>(fldName.c_str()), const_cast<char *>(hIn.c_str()),
          const_cast<char *>(hOut.c_str()), iEnum, iMass, iFlag, firstArg,
          secondArg, cpycomp, Output, hUnits, iUcode, liquidPhase, vaporPhase,
          x3, q, ierr, herr, 10000, 255, 255, 255, 255);
      if (ierr > 0) {
        Napi::Value retValue = Napi::Value::From(env, herr);
        return retValue.ToObject();
      }
      for (int i = 0; i < hOutLength; i++) {
        // std::string temp = propStr[i].As<String>().Utf8Value();
        resVal.Set(propStr[i], Output[i]);
      }
      // printf("%3.2f %3.2f %d %3.2f %f %f %f %f\n", begin_table, step_table,
      // i_tbl, secondArg, Output[0], Output[1], Output[2], Output[3]);
      resArr.Set(i_tbl + 1, resVal);
    }
    // for (int j = 0; j < idx_table; j++)
    // {
    //     for (int i = 0; i < hOutLength; i++)
    //     {
    //         resVal.Set(propArray[i], Output[i + j * idx_table]);
    //     }
    // }
  }

  std::string ulerr;
  bool unloaded_REFPROP = unload_REFPROP(ulerr);
  if (!unloaded_REFPROP) {
    return Napi::String::From(env, ulerr).ToObject();
  }

  /* TODO: No intellisence is provided to end-user. Again, I'm that guy.
   */

  Napi::Object resUnit = Napi::Object::New(env);
  resUnit.Set("T", "K");
  resUnit.Set("P", "Pa");
  resUnit.Set("D", "kg/m3");
  resUnit.Set("VIS", "kg/m-s");
  resUnit.Set("KV", "m2/s");
  resUnit.Set("TCX", "W/mK");
  resUnit.Set("CP", "J/kg-K");
  resUnit.Set("CV", "J/kg-K");
  resUnit.Set("PRANDTL", "-");
  resUnit.Set("H", "J/kg");
  resUnit.Set("W", "m/s");
  resUnit.Set("S", "J/kg-K");
  resUnit.Set("TD", "m2/s");
  resUnit.Set("STN", "N/m");
  resUnit.Set("M", "kg/mol");

  Napi::Object resObj = Napi::Object::New(env);
  resObj.Set("Value", resArr);
  resObj.Set("Unit", resUnit);
  return resObj;
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Refprop::Init(env, exports);

  exports.Set(Napi::String::New(env, "SuperProp"),
              Napi::Function::New(env, AllinOne));
  // exports.Set(Napi::String::New(env, "echoString"), Napi::Function::New(env,
  // EchoString));

  return exports;
}

NODE_API_MODULE(addon, InitAll);