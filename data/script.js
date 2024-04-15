function sendit(theUrl, callback)
{
    console.log(`ok`);
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
    }
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}

var HttpClient = function() {
    this.get = function(aUrl, aCallback) {
        var anHttpRequest = new XMLHttpRequest();
        anHttpRequest.onreadystatechange = function() { 
            if (anHttpRequest.readyState == 4 && anHttpRequest.status == 200)
                aCallback(anHttpRequest.responseText);
        }

        anHttpRequest.open( "GET", aUrl, true );            
        anHttpRequest.send( null );
    }
}
var client = new HttpClient();

function powerCheck(){
    //POWER
    powerState = document.getElementById('coinList').innerHTML;
    if (powerState = "0"){
        document.getElementById('main-power').classList.remove("off");
        document.getElementById('main-power').classList.remove("on");
        document.getElementById('main-power').classList.add("off");
    }
    else{
        document.getElementById('main-power').classList.remove("on");
        document.getElementById('main-power').classList.remove("off");
        document.getElementById('main-power').classList.add("on");
    }
}

function verificarBotoesAtivos(){
    var botoes = document.getElementsByClassName('coin-btn');
    for (var i=0; i< botoes.length; i++ ) {
        console.log(botoes[i].id);
        var currentCoins = document.getElementById('currentCoins').value;
        console.log("currentCoins="+currentCoins);
        if(currentCoins.indexOf(botoes[i].id) != -1){
            console.log('a');
            botoes[i].classList.add("active");
        }
    }

    // CLOCK E TEXTO
    var clockState = document.getElementById('clockState').innerHTML;
    var customText = document.getElementById('customText').innerHTML;

    if(clockState == "1"){
        document.getElementById("inputClockBtn").classList.remove("off");
        document.getElementById("inputClockBtn").classList.add("on");
        document.getElementById("inputClockBtn").checked = true;
    }
    if(customText == "1"){
        document.getElementById("inputStringBtn").classList.remove("off");
        document.getElementById("inputStringBtn").classList.add("on");
        document.getElementById("inputStringBtn").checked = true;
    }

    powerCheck();

}

function remontarBotoes(){
    //remontar botoes
    var coinList = document.getElementById('coinList').value;
    var currentCoins = document.getElementById('currentCoins').value;
    document.getElementById("main").innerHTML = "";
    var symbol = "";
    var active = "";
    for(var i=0;i<coinList.length;i++){
        if(coinList[i] != ";"){
            symbol += coinList[i];  
        }
        else{
            if(currentCoins.indexOf(symbol) != -1){
                active = "active";
            }
            var node = '<div class="coin-btn ' + active + '" id="' + symbol + '">' + symbol + '</div>';
            document.getElementById("main").innerHTML += node ;
            symbol = "";
            active = "";
        }
    }
}

window.onload = function() {

    //Get a reference to the link on the page
    // with an id of "mylink"
    var mainPower = document.getElementById("main-power");
    var mainBtc = document.getElementById("main-btc");
    var mainGear = document.getElementById("main-gear");
    var mainPlus = document.getElementById("main-plus");
    //Set code to run when the link is clicked
    // by assigning a function to "onclick"
    mainBtc.onclick = function() {
        document.getElementById("slides").classList.add("invisible");
        document.getElementById("addCoin").classList.add("invisible");
        document.getElementById("main").classList.remove("invisible");

        remontarBotoes();

        return false;
    }
    mainGear.onclick = function() {
        document.getElementById("main").classList.add("invisible");
        document.getElementById("addCoin").classList.add("invisible");
        document.getElementById("slides").classList.remove("invisible");
        return false;
    }
    mainPlus.onclick = function() {
        document.getElementById("main").classList.add("invisible");
        document.getElementById("slides").classList.add("invisible");
        document.getElementById("addCoin").classList.remove("invisible");
        return false;
    }
    mainPower.onclick = function(e) {
        //DESLIGAR
        if(mainPower.className.indexOf('on') != -1) {
            mainPower.classList.remove("on");
            mainPower.classList.add("off");
            client.get('/get?mainPower=0', function(response) { //CHAMANDO O GET
                // do something with response
            });
            console.log("OFF");
        }
        //LIGAR
        else{
            mainPower.classList.remove("off");
            mainPower.classList.add("on");
            client.get('/get?mainPower=1', function(response) { //CHAMANDO O GET
                // do something with response
            });
            console.log("ON");
        }
        powerCheck();

    }

    // SLIDERSSSSS
    // BRILHO
    var slideBrightness = document.getElementById('slideBrightness');
    slideBrightness.onchange = function() {
        var brightness = this.value;
        client.get('/get?slideBrightness='+brightness, function(response) { //CHAMANDO O GET
            // do something with response
        });
    }
    //VELOCIDADE
    var slideSpeed = document.getElementById('slideSpeed');
    slideSpeed.onchange = function() {
        var speed = this.value;
        client.get('/get?slideSpeed='+speed, function(response) { //CHAMANDO O GET
            // do something with response
        });
    }

    verificarBotoesAtivos();
    document.getElementById("refreshCoinList").click();
    document.getElementById("main-btc").click();

}


// CLICK EVENTS
document.onclick = function(e){
    var target = e.target || e.srcElement;

    //  O ESQUEMA É PEGAR O VALOR NO CAMPO, E MANDAR UM GET COM ESSE VALOR
    // CLICANDO NO "Adicionar" - add coins
    if (target.id && target.id.indexOf('addCoinBtn') != -1) {
        // console.log("Adicionar");
        var newCoin = document.getElementById('newCoin').value.toUpperCase();
        var coinList = document.getElementById('coinList').value;
        if(coinList.indexOf(newCoin+";") !== -1){
            alert("Moeda já existe");
            return;
        }
        else{
            coinList = coinList + newCoin + ";";
            document.getElementById('coinList').value = coinList;
            console.log(coinList);
            client.get('/get?coinList='+coinList.toUpperCase(), function(response) { //CHAMANDO O GET
                // do something with response
            });
            document.getElementById('newCoin').value = "";
    
            // ADD BOTOES
            var node = '<div class="coin-btn" id="' + newCoin + '">' + newCoin + '</div>';
            document.getElementById("main").innerHTML += node ;
            //ARMEZANAR O HTML NO SPIFFS
            var mainHtml = document.getElementById("main").innerHTML;
            client.get('/get?mainHtml='+mainHtml, function(response) { //CHAMANDO O GET
                // do something with response
            });
            verificarBotoesAtivos();
        }


    }

    // Atualizar coinList
    else if(target.id && target.id.indexOf('refreshCoinList') != -1){
        var coinList = document.getElementById('coinList').value;
        if(coinList.slice(-1) != ";"){
            coinList = coinList + ";";
            if(coinList == ";"){
                document.getElementById('coinList').value = "";
                return;
            }
            else{
                document.getElementById('coinList').value = coinList;
            }

        }
        client.get('/get?coinList='+coinList.toUpperCase(), function(response) { //CHAMANDO O GET
            // do something with response
        });

        // ATUALIZAR PAR DA MOEDA
        var currencyMaster = document.getElementById("currencyMaster").value;
        if(currencyMaster == null || currencyMaster == ""){
            currencyMaster = "USDT";
        }
        client.get('/get?currencyMaster='+currencyMaster, function(response) { //CHAMANDO O GET
            document.getElementById("currencyMasterLabel").innerHTML = "Par da moeda: (" + currencyMaster +")";
        });

        verificarBotoesAtivos();

        remontarBotoes();

    }

    // SAVE WIFI
    else if(target.id && target.id.indexOf('saveWifi') != -1) {
        var wifiSSID = document.getElementById("ssid").value;
        var wifiPassword = document.getElementById("password").value;

        client.get('/get?wifiSSID='+wifiSSID, function(response) { //CHAMANDO O GET
            client.get('/get?wifiPassword='+wifiPassword, function(response) { //CHAMANDO O GET
                client.get('/reboot', function(response) { //CHAMANDO O GET
                    document.getElementById("rapitAP").innerHTML(`<br>Reiniciando o Cripto Rapit...<br>
                        Agora conecte-se à sua rede WiFi e use o IP que aparece no display!<br>
                    `)
                });
            });
        });  
    }


    // SAVE SETTINGS
    else if(target.id && target.id.indexOf('saveSettings') != -1) {
        var apiMaster = document.getElementById("apiMaster").value;
        var keyMaster = document.getElementById("keyMaster").value;

        client.get('/get?apiMaster='+apiMaster, function(response) { //CHAMANDO O GET
            document.getElementById("apiMasterLabel").innerHTML = "API Master: (" + apiMaster +")";
        });

        client.get('/get?keyMaster='+keyMaster, function(response) { //CHAMANDO O GET
            document.getElementById("keyMasterLabel").innerHTML = "Key: (" + keyMaster +")";
        });
    }

    // COIN ON/OFF
    else if (target.className && target.className.indexOf('coin-btn') != -1) {
        if(target.className && target.className.indexOf('active') != -1){
            target.classList.remove("active");
            var id = target.id;
            var currentCoins = document.getElementById('currentCoins').value;
            currentCoins = currentCoins.replace(id+";", "");
            client.get('/get?currentCoins='+currentCoins, function(response) { //CHAMANDO O GET
                // do something with response
            });
        }
        else{
            target.classList.add("active");
            //console.log(id);
            var id = target.id;
            var currentCoins = document.getElementById('currentCoins').value;
            currentCoins = currentCoins + id + ";";
            client.get('/get?currentCoins='+currentCoins, function(response) { //CHAMANDO O GET
                // do something with response
            });
        }
        document.getElementById('currentCoins').value = currentCoins;

        //console.log(document.getElementById('currentCoins').value);

    }

    // EXIBIR/OCULTAR TEXTO PERSONALIZADO
    else if (target.id && target.id.indexOf('inputStringBtn') != -1) {
        // EXIBIR
        if(target.className.indexOf('off') != -1){
            console.log('Exibi Texto');
            target.classList.remove('off');
            target.classList.add('on');
            // document.getElementById("inputStringBtn").innerHTML = "TEXTO ON";
            var customText = document.getElementById("inputString").value;
            client.get('/get?inputString='+customText, function(response) { //CHAMANDO O GET
                document.getElementById("inputStringLabel").innerHTML = "Texto Personalizado: (" + customText +")";
            });
            client.get('/get?customText=1', function(response) { //CHAMANDO O GET
                // do something with response
            });

        }
        else{
            console.log('Ocultei Texto');
            target.classList.remove('on');
            target.classList.add('off');
            // document.getElementById("inputStringBtn").innerHTML = "TEXTO OFF";
            client.get('/get?customText=0', function(response) { //CHAMANDO O GET
                // do something with response
            });
        }
    }
    // RELOGIO CLOCK ON/OFF
    else if (target.id && target.id.indexOf('inputClockBtn') != -1) {
        // EXIBIR
        if(target.className.indexOf('off') != -1){
            target.classList.remove('off');
            target.classList.add('on');

            // SET SPEED
            client.get('/get?slideSpeed=20', function(response) { //CHAMANDO O GET
                // do something with response
            });

            // TURN ON
            client.get('/get?clockState=1', function(response) { //CHAMANDO O GET
                // do something with response
            });
            console.log("clock on");
        }
        else{
            target.classList.remove('on');
            target.classList.add('off');

            // SET SPEED BACK
            var speed = document.getElementById('slideSpeed').value;

            client.get('/get?slideSpeed='+speed, function(response) { //CHAMANDO O GET
                // do something with response
            });

            // TURN OFF
            client.get('/get?clockState=0', function(response) { //CHAMANDO O GET
                // do something with response
            });
            console.log("clock off");
        }
    }

}
