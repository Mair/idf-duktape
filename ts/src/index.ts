
let x: number;
print("hello world");

const MODE_OUTPUT = 2;
config_gpio(2,MODE_OUTPUT);
setInterval( () => {
    print("toggle pin " + x % 2);
    set_gpio(2, x % 2);
    x++;
}, 1500);

