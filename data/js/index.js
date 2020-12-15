function showText(){
    //String text = "" = $('#text').val();
    var text = $('#text').val();
    console.log(text);
    $.post("/text", { text: text });
}
  
  