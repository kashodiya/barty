
var stn = '';
var urlTemplate = "https://bart-kashodiya.c9users.io/api/etd?orig=";

var key = "ZQ4M-PB6M-9VJT-DWE9";
var bartApiUrl = "http://api.bart.gov/api/etd.aspx?cmd=etd&key=" + key + "&orig=";

Pebble.addEventListener('ready', function(e) {
  console.log('JavaScript app ready and running! Sending message...');

  stn = localStorage.getItem('stn');
  if (!stn) {
    stn = 'FRMT';
    console.log('Setting default stn:', stn);
  }else{
    console.log('Found stn from localStorage', stn);
  }
  sendMessageTest();
});

function getPosition(str, m, i, begining) {
    var txt = str.split(m, i).join(m);
    if(str.length === txt.length) return -1;
    var p = begining ? m.length : 0;
    return str.split(m, i).join(m).length + p;
}

function getTxtOfEle(x, ele, instance){
    var p = getPosition(x, "<" + ele + ">", instance, true);
    if(p == -1) return "";
    var p1 = getPosition(x, "</" + ele + ">", instance, false);
    if(p1 == -1) return "";
    var ans = x.substr(p, p1 - p);
    return ans;
}


function countEle(xml, ele){
  var re = new RegExp("<" + ele + ">", "g");
  var count = (xml.match(re) || []).length;
  return count;
}

function readElement(xml, re) {
  var m = re.exec(xml);
  if (m===null) return "";
  return m[1];
}


function sendMessageTest() {
  var transactionId = Pebble.sendAppMessage( { '0': 42, '1': 'String value' },
    function(e) {
      console.log('Successfully delivered message with transactionId='
        + e.data.transactionId);
    },
    function(e) {
      console.log('Unable to deliver message with transactionId='
        + e.data.transactionId
        + ' Error is: ' + JSON.stringify(e, null, 2));
    }
  );
}

var appKeys = {
  'KeyInit': "0",
  'KeyStn': "1",
  'KeyEtd': "2"
};

function xhrWrapper(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(xhr);
  };
  xhr.open(type, url);
  xhr.send();
};

function getEtd(stnAbbr) {
  var url = bartApiUrl + stnAbbr;
  console.log('Fetching URL:', url);

  xhrWrapper(url, 'GET', function(req) {
    console.log('Got API response!', req.response);
    if(req.status == 200) {

      var name = readElement(req.responseText,/<name>(.+?)<\/name>/g);
      var abbr = readElement(req.responseText,/<abbr>(.+?)<\/abbr>/g);
      console.log("name:", name);
      console.log("abbr:", abbr);

      var etdCount = countEle(req.responseText, "etd");
      console.log("# of etd:", etdCount);
      var etdLines = [];
      for(var i = 1; i <= etdCount; i++){
        var etdXml = getTxtOfEle(req.responseText, "etd", i);
        console.log("etd:", i);
        console.log(etdXml);
        var etdName = readElement(etdXml, /<destination>(.+?)<\/destination>/g);
        var etdAbbr = readElement(etdXml, /<abbreviation>(.+?)<\/abbreviation>/g);
        var minutes = readElement(etdXml, /<minutes>(.+?)<\/minutes>/g);
        if(minutes === "Leaving") minutes = "0";
        var length = readElement(etdXml, /<length>(.+?)<\/length>/g);
        console.log("destination", etdName);
        console.log("abbreviation", etdAbbr);
        console.log("minutes", minutes);
        console.log("length", length);
        etdLines.push(etdAbbr + " " + minutes + "m " + length + "c");
      }

      var etdStr = "\n" + name + "\n===========\n" + etdLines.join("\n")

      var transactionId = Pebble.sendAppMessage( { "2": etdStr },
        function(e) {
          console.log('Successfully delivered message with transactionId='
            + e.data.transactionId);
        },
        function(e) {
          console.log('Unable to deliver message with transactionId='
            + e.data.transactionId
            + ' Error is: ' + JSON.stringify(e, null, 2));
        }
      );

    } else {
      console.log('owm-weather: Error fetching data (HTTP Status: ' + req.status + ')');
    }
  }.bind(this));
};

function getEtdOld(stnAbbr) {
  var url = urlTemplate + stnAbbr;
  console.log('Fetching URL:', url);

  xhrWrapper(url, 'GET', function(req) {
    console.log('Got API response!');
    if(req.status == 200) {
      var serverResponse = JSON.parse(req.response);
      console.log("Response:", JSON.stringify(serverResponse, null, 2));
      sendMessageToWatch(serverResponse);
    } else {
      console.log('owm-weather: Error fetching data (HTTP Status: ' + req.status + ')');
    }
  }.bind(this));
};

function sendMessageToWatch(serverResponse) {
  var etdStr = serverResponse.dest.reduce(function(p, d){
    return p + "\n" + d.destination + " " + d.minutes + "m " + d.length + "c";
  }, "\n" +serverResponse.orig + "\n===========");

  var key = appKeys.KeyEtd.toString();
  var key = appKeys.KeyEtd;
  console.log("Key for KeyEtd:", key);
  console.log("Value for KeyEtd = ", etdStr);

  var transactionId = Pebble.sendAppMessage( { "2": etdStr },
    function(e) {
      console.log('Successfully delivered message with transactionId='
        + e.data.transactionId);
    },
    function(e) {
      console.log('Unable to deliver message with transactionId='
        + e.data.transactionId
        + ' Error is: ' + JSON.stringify(e, null, 2));
    }
  );
}


Pebble.addEventListener('appmessage', function(e) {
  console.log('message received from watch: ', JSON.stringify(e, null, 2) );
  if (e.payload[appKeys['KeyStn']]) {
    stn = e.payload[appKeys['KeyStn']];
    console.log('Storing stn in localStorage', stn);
    localStorage.setItem('stn', stn);
    //fetchStockQuote(symbol, false);
    getEtd(stn);
  }
});
