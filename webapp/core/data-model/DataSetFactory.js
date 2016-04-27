var DataSetError = require('./../Exceptions').DataSetError;
var DataSetDcp = require('./DataSetDcp');
var DataSetOccurrence = require('./DataSetOccurrence');
var DataSetMonitored = require('./DataSetMonitored');


function build(params) {
  // checking if there a position key for DCP
  if (params.hasOwnProperty("position"))
    return new DataSetDcp(params);
  // checking for monitored object
  if (params.hasOwnProperty('time_column') && params.hasOwnProperty('geometry_column'))
    return new DataSetMonitored(params);

  // todo: validate it - occurrence}
  if (params.hasOwnProperty('id') && params.id > 0) {
    return new DataSetOccurrence(params);
  }
  
  throw new DataSetError("Invalid data set specified");
}

module.exports = {
  build: build
};