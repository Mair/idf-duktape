declare function print(message: string): void;
declare function setInterval(cb: () => void, time: number): void;
declare function setTimeout(cb: () => void, time: number): void;
declare function config_gpio(pin: number, mode: number): void;
declare function set_gpio(pin: number, value: number): void;


interface Packet {
  source: string;
  destination: string;
  bssid: string;
}

