var mode = 0;

var diakritika = "ščřžěňďťůĚŠČŘŽĚŇĎŤŮ";
var noDiakritika = "scrzendtuESCRZENDTU";

function convertDiakritika(str){
    var result = "";
    for(var i=0;i<str.length;i++){
        var c = diakritika.search(str.charAt(i));
        if(c >= 0){
            result+=noDiakritika.charAt(c);
        }
        else{
            result+=str.charAt(i);
        }
    }
    return result;   
}

function showText(){
    mode = 0;
    var text = $('#text').val();
    console.log(convertDiakritika(text));
    text = convertDiakritika(text);
    console.log(mode);
    $.post("/text", { text: text });
}

function showClock(){
    $.post("/clock");
    mode = 1;
    console.log(mode);
}




  