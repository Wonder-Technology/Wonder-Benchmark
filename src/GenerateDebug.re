open PerformanceTestDataType;

open Node;

open Js.Promise;

let _getType = () => "target";

let buildDebugHtmlFileName = (testName, caseName) =>
  GenerateDebugFileUtils.buildDebugHtmlFileName(testName, caseName, _getType());

let _removeTargetDebugFiles = (targetAbsoluteFileDir) =>
  GenerateDebugFileUtils.removeDebugFiles(targetAbsoluteFileDir);

let removeFiles = (debugFileDir, copiedScriptFileDir: option(string)) => {
  debugFileDir |> _removeTargetDebugFiles;
  GenerateBaseDebugUtils.removeDebugFiles(copiedScriptFileDir)
};

let generateHtmlFiles =
    (targetAbsoluteFileDir: string, {commonData} as performanceTestData, compareResultList) =>
  compareResultList
  |> List.iter(
       ((_, (testName, {name, bodyFuncStr}, _, _, _))) => {
         GenerateDebugFileUtils.generate(
           targetAbsoluteFileDir,
           (
             testName,
             name,
             ScriptFileUtils.getAllScriptFilePathList(commonData),
             bodyFuncStr,
             _getType()
           ),
           performanceTestData
         );
         /* GenerateBaseDebugUtils.isGenerateBaseDebugData(performanceTestData) ?
            GenerateBaseDebugUtils.generateBaseDebugFile(
              targetAbsoluteFileDir,
              (testName, name, scriptFilePathList, bodyFuncStr),
              performanceTestData
            ) :
            () */
         GenerateBaseDebugUtils.generateBaseDebugFile(
           targetAbsoluteFileDir,
           (testName, name, bodyFuncStr),
           performanceTestData
         )
       }
     );