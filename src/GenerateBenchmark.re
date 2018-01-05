open Js.Promise;

open Node;

open PerformanceTestDataType;

open BenchmarkDataType;

/*
 let _buildTimeCaseStr = (timeTextList, timeList) =>
   Json.Encode.(
     List.fold_left2(
       (list, text, time) => [
         [("name", name |> string), ("time", time |> int)] |> object_ |> Js.Json.stringify,
         ...list
       ],
       timeTextList,
       timeList
     )
     |> Array.of_list
     |> Js.Array.joinWith(",")
   ); */
/* let convertStateJsonToRecord = (page, browser, scriptFilePathList, dataFilePath, config: config) =>
   Json.(
     Decode.{
       config,
       page,
       browser,
       scriptFilePathList,
       dataFilePath,
       name: "",
       caseList: [],
       result: None
     }
   ); */
let _buildDataFilePath = (benchmarkPath, fileName) => Path.join([|benchmarkPath, fileName|]);

let getDataFilePath = (testName, {benchmarkPath}) =>
  _buildDataFilePath(benchmarkPath, BenchmarkDataConverter.buildDataFileName(testName));

/*
 let _generateCaseData = ((caseName, errorRate, timestamp, timeTextList, timeList, memory)) => {
   let timeCaseStr = _buildTimeCaseStr(timeTextList, timeList);
   {j|
            {
                "name": "$caseName",
                "timestamp":$timestamp,
                "time_detail": [
                    $timeCaseStr
                ],
                "memory": $memory,
                "error_rate": $errorRate
            },
        |j}
 }; */
let generate = (browser, {commonData} as performanceTestData) => {
  let {benchmarkPath, execCountWhenGenerateBenchmark} = commonData;
  Measure.measure(browser, execCountWhenGenerateBenchmark, performanceTestData)
  |> then_(
       (resultList) => {
         resultList
         |> List.iter(
              ((testName, resultCaseList)) => {
                let filePath =
                  _buildDataFilePath(
                    benchmarkPath,
                    BenchmarkDataConverter.buildDataFileName(testName)
                  );
                WonderCommonlib.DebugUtils.log({j|generate benchmark:$filePath...|j}) |> ignore;
                BenchmarkDataConverter.convertToJsonStr(testName, resultCaseList)
                |> NodeExtend.writeFile(filePath)
              }
            );
         browser |> resolve
       }
     )
};