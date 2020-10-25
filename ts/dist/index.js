var x = 0;
print("hello world");
var MODE_OUTPUT = 2;
config_gpio(2, MODE_OUTPUT);
setInterval(function () {
    print("hello world");
    print("toggle pin " + x % 2);
    set_gpio(2, x % 2);
    x++;
}, 1500);
