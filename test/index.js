const path = require("path");
const osenv = process.platform;
const addonPath = path.join(
  __dirname,
  "..",
  osenv === "win32"
    ? "windows-refprop.node"
    : osenv === "linux"
    ? "docker-refprop.node"
    : "apple-refprop.node"
);
const dllPath = path.join(__dirname, "..", "refprop");
const dllName =
  osenv === "win32"
    ? "REFPRP64.DLL"
    : osenv === "linux"
    ? "librefprop.so"
    : "librefprop.dylib";

const refprop = require(addonPath);
const should = require("should");
const assert = require("assert");

const [pureFld, mixtureFld, getBasic, superGet, getProp, getTable] = [
  false,
  true,
  false,
  true,
  true,
  false,
];

const propertyArray = [
  "T",
  "P",
  "D",
  "VIS",
  "KV",
  "TCX",
  "CP",
  "CV",
  "PRANDTL",
  "H",
  "W",
  "S",
  "TD",
  "M",
];

function SummarizedProp(
  fluidType,
  fluidName,
  propertyPair,
  firstArg,
  secondArg,
  outputProp,
  isSuper,
  isTable
) {
  return refprop.GetProp(
    dllPath,
    dllName,
    fluidType,
    fluidName,
    propertyPair,
    firstArg,
    secondArg,
    outputProp,
    isSuper,
    isTable
  );
}

// Input Units
// T : K    // P : Pa     // D : kg/m3    // H : J/kg     // S : J/kgK
// W : m/s  // KV  : m2/s // VIS : Pa s   // KTH : W/mK   // STN : N/m    // M : kg/mol

describe("refprop", function () {
  it("should compute single-phase states specifed in hOut for pure fluids", function () {
    const Qarr = [0, 0.2, 1.0];
    const Pconst = 208644.5;
    for (let iQ = 0; iQ < 3; iQ++) {
      const resultObj = SummarizedProp(
        pureFld,
        "R134a",
        "PQ",
        Pconst,
        Qarr[iQ],
        "T,P,D,VIS,KV,TCX,CP,CV,PRANDTL,H,W,S,TD,M,PC",
        superGet,
        getProp
      );
      const result = resultObj.Value[1];
      result.should.be.Object;
      result.should.have.properties(propertyArray);
    }
  });

  it("should compute single-phase states specifed in hOut for pure fluids", function () {
    const result = SummarizedProp(
      pureFld,
      "Nitrogen",
      "TP",
      273.15,
      101.3e3,
      "T,P,D,VIS,KV,TCX,CP,CV,PRANDTL,H,W,S,TD,M",
      superGet,
      getProp
    ).Value[1];
    result.should.be.Object;
    result.should.have.properties(propertyArray);

    result.T.should.be.eql(273.15);
    result.P.should.be.approximately(101.3e3, 0.1e3);
    result.H.should.be.approximately(283.23e3, 0.01e3);
    result.D.should.be.approximately(1.2501, 0.0001);
    result.S.should.be.approximately(6.7442e3, 0.1);
  });

  it("should compute single-phase states specifed in default for pure fluids", function () {
    const resultObj = SummarizedProp(
      pureFld,
      "R134a",
      "TP",
      173.15,
      101.3e3,
      "",
      getBasic,
      getProp
    );
    const result = resultObj.Value[1];
    result.should.be.Object;
    result.should.have.properties(propertyArray);

    result.T.should.be.eql(173.15);
    result.P.should.be.approximately(101.3e3, 0.1e3);
    result.H.should.be.approximately(754.08e2, 0.01e2);
    result.D.should.be.approximately(1.5825e3, 0.0001e3);
  });

  it("should compute single-phase states specifed in hOut for mixture", function () {
    const result = SummarizedProp(
      mixtureFld,
      "R410a",
      "TP",
      273.15,
      101.3e3,
      "T,P,D,VIS,KV,TCX,CP,CV,PRANDTL,H,W,S,TD,M",
      superGet,
      getProp
    ).Value[1];
    result.should.be.Object;
    result.should.have.properties(propertyArray);

    result.T.should.be.eql(273.15);
    result.P.should.be.approximately(101.3e3, 0.1e3);
    result.H.should.be.approximately(440.79e3, 0.01e3);
    result.D.should.be.approximately(3.2981, 0.0001);
    result.S.should.be.approximately(2.169e3, 0.1);
  });

  it("should compute single-phase states specifed in default for mixture", function () {
    const resultObj = SummarizedProp(
      mixtureFld,
      "R410a",
      "TP",
      173.15,
      101.3e3,
      "",
      getBasic,
      getProp
    );

    const result = resultObj.Value[1];
    result.should.be.Object;
    result.should.have.properties(propertyArray);

    result.T.should.be.eql(173.15);
    result.P.should.be.approximately(101.3e3, 0.1e3);
    result.H.should.be.approximately(61.973e3, 0.001e3);
    result.D.should.be.approximately(1.4948e3, 0.0001e3);
  });

  it("should compute single-phase states specifed in default for mixture with PQ", function () {
    const resultObj = SummarizedProp(
      mixtureFld,
      "R410a",
      "PQ",
      101.3e3,
      0,
      "",
      getBasic,
      getProp
    );

    const result = resultObj.Value[1];
    result.should.be.Object;
    result.should.have.properties(propertyArray);

    result.P.should.be.approximately(101.3e3, 0.1e3);
    result.T.should.be.approximately(221.7, 0.01);
    result.H.should.be.approximately(127.55e3, 0.01e3);
    result.D.should.be.approximately(1.3497e3, 0.0001e3);
  });

  it("should compute table of mixture for 11 step with PQ", function () {
    // You MUST use secondArg as Begin/Step/End Parameter
    const firstArg = 801.3e3;
    const Tbegin = 0.0,
      Tstep = 0.1,
      Tnum = 11;
    const hOut = "T,P,D,VIS";
    const resultObj = SummarizedProp(
      mixtureFld,
      "R410a",
      "PQ",
      firstArg,
      Tbegin,
      hOut,
      superGet,
      getTable,
      Tbegin,
      Tstep,
      Tnum
    );

    let errorFlag = true;
    Object.keys(resultObj).forEach((elem) => {
      if (elem === "Value") {
        errorFlag = false;
      }
    });

    if (errorFlag == true) {
      const result = resultObj.filter((elem) => {
        return elem !== null;
      });
    } else {
      const result = resultObj.Value.filter((elem) => {
        return elem !== null;
      });

      result.forEach((elem, i) => {
        elem.should.be.Object;
        elem.should.have.properties(["T", "P"]);
      });
    }
  });

  it("should throw an error for invalid fluids", function () {
    assert.equal(
      refprop.GetProp(
        dllPath,
        dllName,
        mixtureFld,
        "LiBr;0.7; Water;0.3;",
        "TQ",
        "T,P,D,VIS,KV,TCX,CP,CV,PRANDTL,H,W,S,TD,STN,M",
        220,
        0.5
      ).Value,
      undefined
    );
  });

  it("should compute table of pure for 50 step", function () {
    // You MUST use secondArg as Begin/Step/End Parameter
    const firstArg = 101.3e3;
    const Tbegin = 273.15,
      Tstep = 0.1,
      Tnum = 50;
    const hOut = "T,P";
    const resultObj = refprop.GetPropTable(
      dllPath,
      dllName,
      pureFld,
      "Nitrogen",
      "PT",
      firstArg,
      273.15,
      hOut,
      superGet,
      getTable,
      Tbegin,
      Tstep,
      Tnum
    );

    const result = resultObj.Value.filter((elem) => {
      return elem !== null;
    });

    resultObj.forEach((elem, i) => {
      elem.should.be.Object;
      elem.should.have.properties(["T", "P"]);
      elem.T.should.be.approximately(Tbegin + Tstep * i, 0.01);
      elem.P.should.be.approximately(firstArg, 0.01);
    });
  });

  it("should compute table of mixture for 50 step", function () {
    // You MUST use secondArg as Begin/Step/End Parameter
    const firstArg = 101.3e3;
    const Tbegin = 273.15,
      Tstep = 0.1,
      Tnum = 50;
    const hOut = "T,P,D,VIS";
    const resultObj = refprop.GetPropTable(
      dllPath,
      dllName,
      mixtureFld,
      "R410a",
      "PT",
      firstArg,
      273.15,
      hOut,
      superGet,
      getTable,
      Tbegin,
      Tstep,
      Tnum
    );

    const result = resultObj.Value.filter((elem) => {
      return elem !== null;
    });

    result.forEach((elem, i) => {
      elem.should.be.Object;
      elem.should.have.properties(["T", "P"]);
      elem.T.should.be.approximately(Tbegin + Tstep * i, 0.01);
      elem.P.should.be.approximately(firstArg, 0.01);
    });
  });

  it("should compute states for custom mixtures", function () {
    const result = refprop.GetPropTable(
      dllPath,
      dllName,
      pureFld,
      "WATER;0.3; ARGON;0.7;",
      "TP",
      "T,P,D,VIS,KV,TCX,CP,CV,PRANDTL,H,W,S,TD,M",
      293.15,
      101300
    );

    result.Value.should.be.Object;
    result.Value.should.have.properties(propertyArray);
  });
});
