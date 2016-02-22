var stn = '';
var urlTemplate = "https://bart-kashodiya.c9users.io/api/etd?orig=";

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
