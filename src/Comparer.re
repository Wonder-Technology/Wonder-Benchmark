open Node;

open WonderCommonlib.NodeExtend;

open Performance;

open Root;

open Contract;

open Js.Promise;

open BenchmarkDataType;

open PerformanceTestDataType;

let _getRange = (target, errorRate) => (
  (target |> Number.intToFloat)
  *. (100. -. (errorRate |> Number.intToFloat))
  /. 100.0
  |> Js.Math.floor,
  (target |> Number.intToFloat)
  *. (100. +. (errorRate |> Number.intToFloat))
  /. 100.0
  |> Js.Math.ceil
);

let _isInRange = (actual, (min, max)) => actual >= min && actual <= max;

let _getDiffPercentStr = (diff) =>
  switch diff {
  | value when value >= 0 => {j|+$diff%|j}
  | value =>
    let diff = value |> Js.Math.abs_int;
    {j|-$diff%|j}
  };

let _getDiffPercent = (actualValue, targetValue) => {
  requireCheck(
    () =>
      Contract.Operators.(test("targetValue should >= 0", () => assertGte(Int, targetValue, 0)))
  );
  switch targetValue {
  | 0 => 1000
  | _ =>
    (actualValue - targetValue |> Number.intToFloat)
    /. (targetValue |> Number.intToFloat)
    *. 100.0
    |> Js.Math.unsafe_round
  }
};

let _compareMemory =
    (actualMemory, benchmarkMemory, errorRate, (failMessage, diffTimePercentList, passedTimeList)) => {
  let (minMemory, maxMemory) as rangeData = _getRange(benchmarkMemory, errorRate);
  switch (_isInRange(actualMemory, rangeData)) {
  | false =>
    let diff = _getDiffPercent(actualMemory, benchmarkMemory);
    let diffStr = _getDiffPercentStr(diff);
    (
      failMessage
      ++ {j|expect memory to in [$minMemory, $maxMemory], but actual is $actualMemory. the diff is $diffStr\n|j},
      diffTimePercentList,
      passedTimeList,
      diff
    )
  | true => (failMessage, diffTimePercentList, passedTimeList, 0)
  }
};

let _isTimePassed = (passedTimeList, index) =>
  switch passedTimeList {
  | None => false
  | Some(passedTimeList) => List.nth(passedTimeList, index)
  };

let _compareTime =
    (
      actualTimeList: list(int),
      actualTimeTextList,
      benchmarkTimeList,
      passedTimeList,
      errorRate,
      failMessage
    ) => {
  let index = ref((-1));
  List.fold_left2(
    ((failMessage, diffList, newPassedTimeList), (actualTime, actualTimeText), benchmarkTime) => {
      index := index^ + 1;
      _isTimePassed(passedTimeList, index^) ?
        (failMessage, diffList, [true, ...newPassedTimeList]) :
        {
          let (minTime, maxTime) = _getRange(benchmarkTime, errorRate);
          switch (_isInRange(actualTime, (minTime, maxTime))) {
          | false =>
            let diff = _getDiffPercent(actualTime, benchmarkTime);
            let diffStr = _getDiffPercentStr(diff);
            (
              failMessage
              ++ {j|expect time:$actualTimeText to in [$minTime, $maxTime], but actual is $actualTime. the diff is $diffStr\n|j},
              diffList @ [diff],
              [false, ...newPassedTimeList]
            )
          | true => (failMessage, diffList, [true, ...newPassedTimeList])
          }
        }
    },
    (failMessage, [], []),
    List.combine(actualTimeList, actualTimeTextList),
    benchmarkTimeList
  )
};

let _isFail = (failMessage) => failMessage |> Js.String.length > 0;

let isPass = (failList) => failList |> List.length === 0;

let getFailText = (failList) =>
  failList
  |> List.fold_left(
       (text, (failMessage, (testName, {name}: case, _, _, _))) =>
         text ++ PerformanceTestDataUtils.buildCaseTitle(testName, name) ++ failMessage,
       ""
     );

let getFailCaseText = (failList) =>
  failList
  |> List.map(((failMessage, (testName, {name}: case, _, _, _))) => (testName, name, failMessage));

let _getPassedTimeList = (passedTimeListMap, testName, caseName) : option(list(bool)) =>
  switch (passedTimeListMap |> WonderCommonlib.HashMapSystem.get(testName)) {
  | None => None
  | Some(map) =>
    switch (map |> WonderCommonlib.HashMapSystem.get(caseName)) {
    | None => None
    | Some(timeList) => Some(timeList)
    }
  };

let compare =
  [@bs]
  (
    (
      browser,
      allScriptFilePathList,
      passedTimeListMap,
      {commonData, testDataList} as performanceTestData
    ) =>
      Measure.measure(
        browser,
        commonData.execCountWhenTest,
        allScriptFilePathList,
        performanceTestData
      )
      |> then_(
           (resultList: list((string, 'a))) =>
             resultList
             |> List.fold_left(
                  (compareResultList, (actualTestName, actualResultCaseList)) =>
                    switch (
                      GenerateBenchmark.getBenchmarkData(actualTestName, commonData.benchmarkPath)
                    ) {
                    | (benchmarkTestName, _) when benchmarkTestName !== actualTestName =>
                      ExceptionHandleSystem.throwMessage(
                        {j|benchmarkTestName:$benchmarkTestName should === actualTestName:$actualTestName|j}
                      )
                    | (_, benchmarkResultCaseList) =>
                      List.fold_left2(
                        (
                          failList,
                          (
                            actualCaseName,
                            actualErrorRate,
                            actualTimestamp,
                            actualTimeTextList,
                            actualTimeList,
                            actualMemory,
                            actualCase
                          ),
                          (
                            benchmarkCaseName,
                            benchmarkErrorRate,
                            benchmarkTimestamp,
                            benchmarkTimeTextList,
                            benchmarkTimeList,
                            benchmarkMemory
                          )
                        ) =>
                          actualCaseName !== benchmarkCaseName ?
                            ExceptionHandleSystem.throwMessage(
                              {j|actual caseName:$actualCaseName should === benchmark caseName:$benchmarkCaseName|j}
                            ) :
                            actualErrorRate !== benchmarkErrorRate ?
                              ExceptionHandleSystem.throwMessage(
                                {j|actual errorRate:$actualErrorRate should === benchmark errorRate:$benchmarkErrorRate|j}
                              ) :
                              (
                                switch (
                                  ""
                                  |> _compareTime(
                                       actualTimeList,
                                       actualTimeTextList,
                                       benchmarkTimeList,
                                       _getPassedTimeList(
                                         passedTimeListMap,
                                         actualTestName,
                                         actualCaseName
                                       ),
                                       actualErrorRate
                                     )
                                  |> _compareMemory(actualMemory, benchmarkMemory, actualErrorRate)
                                ) {
                                | (
                                    failMessage,
                                    diffTimePercentList,
                                    passedTimeList,
                                    diffMemoryPercent
                                  )
                                    when _isFail(failMessage) => [
                                    (
                                      /* PerformanceTestDataUtils.buildCaseTitle(
                                           actualTestName,
                                           actualCaseName
                                         )
                                         ++ failMessage, */
                                      failMessage,
                                      (
                                        actualTestName,
                                        actualCase,
                                        diffTimePercentList,
                                        passedTimeList,
                                        diffMemoryPercent
                                      )
                                    ),
                                    ...failList
                                  ]
                                | _ => failList
                                }
                              ),
                        [],
                        actualResultCaseList,
                        benchmarkResultCaseList
                        |> List.filter(
                             ((benchmarkCaseName, _, _, _, _, _)) =>
                               actualResultCaseList
                               |> List.exists(
                                    ((actualCaseName, _, _, _, _, _, _)) =>
                                      actualCaseName === benchmarkCaseName
                                  )
                           )
                      )
                    },
                  []
                )
             |> resolve
         )
  );