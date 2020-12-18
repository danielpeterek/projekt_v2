var mode = 0;

function showText(){
    mode = 0;
    var text = $('#text').val();
    console.log(text);
    console.log(mode);
    $.post("/text", { text: text });
}

function showClock(){
    $.post("/clock");
    mode = 1;
    console.log(mode);
}
  
  