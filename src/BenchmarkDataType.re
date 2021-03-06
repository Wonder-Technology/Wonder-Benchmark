/* type caseTimeItem = {
  name: string,
  time: int
};

type caseItem = {
  name: string,
  time: list(caseTimeItem),
  memory: int,
  errorRate: option(int)
};

type result = {
  name: string,
  errorRate: int,
  timestamp: int,
  timeTextArray: array(string),
  timeArray: array(int),
  memory: int
};

type config = {
  isClosePage: bool,
  execCount: int
};

type state = {
  config,
  page: WonderBsPuppeteer.Page.t,
  browser: WonderBsPuppeteer.Browser.t,
  scriptFilePathList: list(string),
  dataFilePath:string,
  name: string,
  caseList: list(caseItem),
  result: option(result)
}; */

type resultTimeData = {
  name:string,
  errorRate: int,
  timestamp: int,
  timeArray: array(int),
  timeTextArray: array(string)
};

/* type funcReturnValue = {. "errorRate": int, "textArray": array(string), "timeArray": array(float)}; */