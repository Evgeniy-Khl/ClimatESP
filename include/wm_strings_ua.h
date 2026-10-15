/**
 * wm_strings_ua.h
 * Ukrainian language strings for WiFiManager
 * Клімат-5.25
 */

#ifndef _WM_STRINGS_UA_H_
#define _WM_STRINGS_UA_H_

#ifndef WIFI_MANAGER_OVERRIDE_STRINGS

#include "wm_consts_en.h" // include constants, tokens, routes

const char WM_LANGUAGE[] PROGMEM = "uk-UA";

const char HTTP_HEAD_START[] PROGMEM = "<!DOCTYPE html>"
"<html lang='uk'><head>"
"<meta name='format-detection' content='telephone=no'>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>"
"<title>{v}</title>";

const char HTTP_SCRIPT[] PROGMEM = "<script>function c(l){"
"document.getElementById('s').value=l.getAttribute('data-ssid')||l.innerText||l.textContent;"
"p = l.nextElementSibling.classList.contains('l');"
"document.getElementById('p').disabled = !p;"
"if(p)document.getElementById('p').focus();};"
"function f() {var x = document.getElementById('p');x.type==='password'?x.type='text':x.type='password';}"
"</script>";

const char HTTP_HEAD_END[] PROGMEM = "</head><body class='{c}'><div class='wrap'>";

const char HTTP_ROOT_MAIN[] PROGMEM = "<h1>{t}</h1><h3>{v}</h3>";

const char * const HTTP_PORTAL_MENU[] PROGMEM = {
"<form action='/wifi'    method='get'><button>Налаштування Wi-Fi</button></form><br/>\n", // MENU_WIFI
"<form action='/0wifi'   method='get'><button>Налаштування Wi-Fi (без пошуку)</button></form><br/>\n", // MENU_WIFINOSCAN
"<form action='/info'    method='get'><button>Інформація про систему</button></form><br/>\n", // MENU_INFO
"<form action='/param'   method='get'><button>Параметри</button></form><br/>\n",//MENU_PARAM
"<form action='/close'   method='get'><button>Закрити</button></form><br/>\n", // MENU_CLOSE
"<form action='/restart' method='get'><button>Перезавантажити</button></form><br/>\n",// MENU_RESTART
"<form action='/exit'    method='get'><button>Вийти</button></form><br/>\n",  // MENU_EXIT
"<form action='/erase'   method='get'><button class='D'>Скинути Wi-Fi</button></form><br/>\n", // MENU_ERASE
"<form action='/update'  method='get'><button>Оновлення ПЗ</button></form><br/>\n",// MENU_UPDATE
"<hr><br/>" // MENU_SEP
};

const char HTTP_PORTAL_OPTIONS[]   PROGMEM = "";
const char HTTP_ITEM_QI[]          PROGMEM = "<div role='img' aria-label='{r}%' title='{r}%' class='q q-{q} {i} {h}'></div>";
const char HTTP_ITEM_QP[]          PROGMEM = "<div class='q {h}'>{r}%</div>";
const char HTTP_ITEM[]             PROGMEM = "<div><a href='#p' onclick='c(this)' data-ssid='{V}'>{v}</a>{qi}{qp}</div>";

const char HTTP_FORM_START[]       PROGMEM = "<form method='POST' action='{v}'>";
const char HTTP_FORM_WIFI[]        PROGMEM = "<label for='s'>Ім'я мережі (SSID)</label><input id='s' name='s' maxlength='32' autocorrect='off' autocapitalize='none' placeholder='{v}'><br/><label for='p'>Пароль</label><input id='p' name='p' maxlength='64' type='password' placeholder='{p}'><input type='checkbox' id='showpass' onclick='f()'> <label for='showpass'>Показати пароль</label><br/>";
const char HTTP_FORM_WIFI_END[]    PROGMEM = "";
const char HTTP_FORM_STATIC_HEAD[] PROGMEM = "<hr><br/>";
const char HTTP_FORM_END[]         PROGMEM = "<br/><br/><button type='submit'>Зберегти</button></form>";
const char HTTP_FORM_LABEL[]       PROGMEM = "<label for='{i}'>{t}</label>";
const char HTTP_FORM_PARAM_HEAD[]  PROGMEM = "<hr><br/>";
const char HTTP_FORM_PARAM[]       PROGMEM = "<br/><input id='{i}' name='{n}' maxlength='{l}' value='{v}' {c}>\n";

const char HTTP_SCAN_LINK[]        PROGMEM = "<br/><form action='/wifi?refresh=1' method='POST'><button name='refresh' value='1'>Оновити список мереж</button></form>";
const char HTTP_SAVED[]            PROGMEM = "<div class='msg'>Збереження налаштувань<br/>Підключення до обраної Wi-Fi мережі...<br />У разі помилки підключіться знову до точки доступу.</div>";
const char HTTP_PARAMSAVED[]       PROGMEM = "<div class='msg S'>Збережено<br/></div>";
const char HTTP_END[]              PROGMEM = "</div></body></html>";
const char HTTP_ERASEBTN[]         PROGMEM = "<br/><form action='/erase' method='get'><button class='D'>Скинути налаштування Wi-Fi</button></form>";
const char HTTP_UPDATEBTN[]        PROGMEM = "<br/><form action='/update' method='get'><button>Оновити</button></form>";
const char HTTP_BACKBTN[]          PROGMEM = "<hr><br/><form action='/' method='get'><button>Назад</button></form>";

const char HTTP_STATUS_ON[]        PROGMEM = "<div class='msg S'><strong>Підключено</strong> до {v}<br/><em><small>IP адреса: {i}</small></em></div>";
const char HTTP_STATUS_OFF[]       PROGMEM = "<div class='msg {c}'><strong>Не підключено</strong> до {v}{r}</div>";
const char HTTP_STATUS_OFFPW[]     PROGMEM = "<br/>Помилка аутентифікації (невірний пароль)";
const char HTTP_STATUS_OFFNOAP[]   PROGMEM = "<br/>Мережу не знайдено";
const char HTTP_STATUS_OFFFAIL[]   PROGMEM = "<br/>Не вдалося підключитися";
const char HTTP_STATUS_NONE[]      PROGMEM = "<div class='msg'>Точку доступу не налаштовано</div>";
const char HTTP_BR[]               PROGMEM = "<br/>";

const char HTTP_STYLE[]            PROGMEM = "<style>"
".c,body{text-align:center;font-family:verdana}div,input,select{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}"
"input,button,select,.msg{border-radius:.3rem;width: 100%}input[type=radio],input[type=checkbox]{width:auto}"
"button,input[type='button'],input[type='submit']{cursor:pointer;border:0;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}"
"input[type='file']{border:1px solid #1fa3ec}"
".wrap {text-align:left;display:inline-block;min-width:260px;max-width:500px}"
"a{color:#000;font-weight:700;text-decoration:none}a:hover{color:#1fa3ec;text-decoration:underline}"
".q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}.q.q-0:after{background-position-x:0}.q.q-1:after{background-position-x:-16px}.q.q-2:after{background-position-x:-32px}.q.q-3:after{background-position-x:-48px}.q.q-4:after{background-position-x:-64px}.q.l:before{background-position-x:-80px;padding-right:5px}.ql .q{float:left}.q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position: 16px 0;"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=');}"
"@media (-webkit-min-device-pixel-ratio: 2),(min-resolution: 192dpi){.q:before,.q:after {"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');"
"background-size: 95px 16px;}}"
".msg{padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777}.msg h4{margin-top:0;margin-bottom:5px}.msg.P{border-left-color:#1fa3ec}.msg.P h4{color:#1fa3ec}.msg.D{border-left-color:#dc3630}.msg.D h4{color:#dc3630}.msg.S{border-left-color: #5cb85c}.msg.S h4{color: #5cb85c}"
"dt{font-weight:bold}dd{margin:0;padding:0 0 0.5em 0;min-height:12px}"
"td{vertical-align: top;}"
".h{display:none}"
"button{transition: 0s opacity;transition-delay: 3s;transition-duration: 0s;cursor: pointer}"
"button.D{background-color:#dc3630}"
"button:active{opacity:50% !important;cursor:wait;transition-delay: 0s}"
"body.invert{background-color:#060606;color:#fff;}"
"body.invert .msg{color:#fff;background-color:#282828;border-top:1px solid #555;border-right:1px solid #555;border-bottom:1px solid #555;}"
"body.invert .q[role=img]{-webkit-filter:invert(1);filter:invert(1);}"
":disabled {opacity: 0.5;}"
"</style>";

#ifndef WM_NOHELP
const char HTTP_HELP[] PROGMEM =
 "<br/><h3>Доступні сторінки</h3><hr>"
 "<table class='table'>"
 "<thead><tr><th>Сторінка</th><th>Призначення</th></tr></thead><tbody>"
 "<tr><td><a href='/'>/</a></td><td>Головне меню</td></tr>"
 "<tr><td><a href='/wifi'>/wifi</a></td><td>Пошук та налаштування Wi-Fi мережі</td></tr>"
 "<tr><td><a href='/wifisave'>/wifisave</a></td><td>Збереження налаштувань</td></tr>"
 "<tr><td><a href='/param'>/param</a></td><td>Сторінка параметрів</td></tr>"
 "<tr><td><a href='/info'>/info</a></td><td>Системна інформація</td></tr>"
 "<tr><td><a href='/u'>/u</a></td><td>Оновлення прошивки (OTA)</td></tr>"
 "<tr><td><a href='/close'>/close</a></td><td>Закрити портал без зупинки</td></tr>"
 "<tr><td>/exit</td><td>Вийти з режиму налаштування</td></tr>"
 "<tr><td>/restart</td><td>Перезавантажити контролер</td></tr>"
 "<tr><td>/erase</td><td>Скинути налаштування Wi-Fi та перезавантажитись</td></tr>"
 "</table>";
#else
const char HTTP_HELP[] PROGMEM = "";
#endif

const char HTTP_UPDATE[] PROGMEM = "Завантаження нової прошивки<br/><form method='POST' action='u' enctype='multipart/form-data' onchange=\"(function(el){document.getElementById('uploadbin').style.display = el.value=='' ? 'none' : 'initial';})(this)\"><input type='file' name='update' accept='.bin,application/octet-stream'><button id='uploadbin' type='submit' class='h D'>Оновити</button></form><small><a href='http://192.168.4.1/update' target='_blank'>* Відкрийте в браузері http://192.168.4.1</a></small>";
const char HTTP_UPDATE_FAIL[] PROGMEM = "<div class='msg D'><strong>Помилка оновлення!</strong><br/>Перезавантажте прилад та спробуйте знову.</div>";
const char HTTP_UPDATE_SUCCESS[] PROGMEM = "<div class='msg S'><strong>Оновлення успішне.</strong><br/>Прилад перезавантажується...</div>";

#ifdef ESP32
	const char HTTP_INFO_esphead[]    PROGMEM = "<h3>esp32</h3><hr><dl>";
	const char HTTP_INFO_chiprev[]    PROGMEM = "<dt>Ревізія чипа</dt><dd>{1}</dd>";
  	const char HTTP_INFO_lastreset[]  PROGMEM = "<dt>Причина останнього перезапуску</dt><dd>CPU0: {1}<br/>CPU1: {2}</dd>";
  	const char HTTP_INFO_aphost[]     PROGMEM = "<dt>Ім'я хоста точки доступу</dt><dd>{1}</dd>";
    const char HTTP_INFO_psrsize[]    PROGMEM = "<dt>Розмір PSRAM</dt><dd>{1} байт</dd>";
	const char HTTP_INFO_temp[]       PROGMEM = "<dt>Температура</dt><dd>{1} C&deg; / {2} F&deg;</dd><dt>Датчик Холла</dt><dd>{3}</dd>";
#else
	const char HTTP_INFO_esphead[]    PROGMEM = "<h3>esp8266</h3><hr><dl>";
	const char HTTP_INFO_fchipid[]    PROGMEM = "<dt>ID Flash-пам'яті</dt><dd>{1}</dd>";
	const char HTTP_INFO_corever[]    PROGMEM = "<dt>Версія ядра</dt><dd>{1}</dd>";
	const char HTTP_INFO_bootver[]    PROGMEM = "<dt>Версія завантажувача</dt><dd>{1}</dd>";
	const char HTTP_INFO_lastreset[]  PROGMEM = "<dt>Причина перезапуску</dt><dd>{1}</dd>";
	const char HTTP_INFO_flashsize[]  PROGMEM = "<dt>Розмір Flash-пам'яті</dt><dd>{1} байт</dd>";
#endif

const char HTTP_INFO_memsmeter[]  PROGMEM = "<br/><progress value='{1}' max='{2}'></progress></dd>";
const char HTTP_INFO_memsketch[]  PROGMEM = "<dt>Пам'ять програми (Sketch)</dt><dd>Використано / Всього байт<br/>{1} / {2}";
const char HTTP_INFO_freeheap[]   PROGMEM = "<dt>Вільна оперативна пам'ять (Heap)</dt><dd>{1} байт</dd>";
const char HTTP_INFO_wifihead[]   PROGMEM = "<br/><h3>Wi-Fi</h3><hr>";
const char HTTP_INFO_uptime[]     PROGMEM = "<dt>Час роботи</dt><dd>{1} хв {2} сек</dd>";
const char HTTP_INFO_chipid[]     PROGMEM = "<dt>ID чипа</dt><dd>{1}</dd>";
const char HTTP_INFO_idesize[]    PROGMEM = "<dt>Розмір Flash</dt><dd>{1} байт</dd>";
const char HTTP_INFO_sdkver[]     PROGMEM = "<dt>Версія SDK</dt><dd>{1}</dd>";
const char HTTP_INFO_cpufreq[]    PROGMEM = "<dt>Частота CPU</dt><dd>{1} МГц</dd>";
const char HTTP_INFO_apip[]       PROGMEM = "<dt>IP точки доступу</dt><dd>{1}</dd>";
const char HTTP_INFO_apmac[]      PROGMEM = "<dt>MAC-адреса точки доступу</dt><dd>{1}</dd>";
const char HTTP_INFO_apssid[]     PROGMEM = "<dt>SSID точки доступу</dt><dd>{1}</dd>";
const char HTTP_INFO_apbssid[]    PROGMEM = "<dt>BSSID</dt><dd>{1}</dd>";
const char HTTP_INFO_stassid[]    PROGMEM = "<dt>SSID домашньої мережі</dt><dd>{1}</dd>";
const char HTTP_INFO_staip[]      PROGMEM = "<dt>IP адреса приладу</dt><dd>{1}</dd>";
const char HTTP_INFO_stagw[]      PROGMEM = "<dt>Основний шлюз</dt><dd>{1}</dd>";
const char HTTP_INFO_stasub[]     PROGMEM = "<dt>Маска підмережі</dt><dd>{1}</dd>";
const char HTTP_INFO_dnss[]       PROGMEM = "<dt>DNS сервер</dt><dd>{1}</dd>";
const char HTTP_INFO_host[]       PROGMEM = "<dt>Ім'я хоста</dt><dd>{1}</dd>";
const char HTTP_INFO_stamac[]     PROGMEM = "<dt>MAC-адреса приладу</dt><dd>{1}</dd>";
const char HTTP_INFO_conx[]       PROGMEM = "<dt>Підключено</dt><dd>{1}</dd>";
const char HTTP_INFO_autoconx[]   PROGMEM = "<dt>Автопідключення</dt><dd>{1}</dd>";

const char HTTP_INFO_aboutver[]     PROGMEM = "<dt>WiFiManager</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutarduino[] PROGMEM = "<dt>Arduino</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutsdk[]     PROGMEM = "<dt>ESP-SDK/IDF</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutdate[]    PROGMEM = "<dt>Дата збірки</dt><dd>{1}</dd>";

const char S_brand[]              PROGMEM = "Клімат-5.25";
const char S_debugPrefix[]        PROGMEM = "*wm:";
const char S_y[]                  PROGMEM = "Так";
const char S_n[]                  PROGMEM = "Ні";
const char S_enable[]             PROGMEM = "Увімкнено";
const char S_disable[]            PROGMEM = "Вимкнено";
const char S_GET[]                PROGMEM = "GET";
const char S_POST[]               PROGMEM = "POST";
const char S_NA[]                 PROGMEM = "Невідомо";
const char S_passph[]             PROGMEM = "********";
const char S_titlewifisaved[]     PROGMEM = "Налаштування збережено";
const char S_titlewifisettings[]  PROGMEM = "Параметри збережено";
const char S_titlewifi[]          PROGMEM = "Налаштування Wi-Fi";
const char S_titleinfo[]          PROGMEM = "Інформація про систему";
const char S_titleparam[]         PROGMEM = "Параметри";
const char S_titleparamsaved[]    PROGMEM = "Параметри збережено";
const char S_titleexit[]          PROGMEM = "Вихід";
const char S_titlereset[]         PROGMEM = "Перезавантаження";
const char S_titleerase[]         PROGMEM = "Скидання";
const char S_titleclose[]         PROGMEM = "Закрити";
const char S_options[]            PROGMEM = "налаштування";
const char S_nonetworks[]         PROGMEM = "Мереж не знайдено. Оновіть список для повторного пошуку.";
const char S_staticip[]           PROGMEM = "Статичний IP";
const char S_staticgw[]           PROGMEM = "Основний шлюз";
const char S_staticdns[]          PROGMEM = "DNS сервер";
const char S_subnet[]             PROGMEM = "Маска підмережі";
const char S_exiting[]            PROGMEM = "Вихід...";
const char S_resetting[]          PROGMEM = "Прилад перезавантажиться за кілька секунд.";
const char S_closing[]            PROGMEM = "Ви можете закрити сторінку, портал продовжить роботу.";
const char S_error[]              PROGMEM = "Виникла помилка";
const char S_notfound[]           PROGMEM = "Сторінку не знайдено\n\n";
const char S_uri[]                PROGMEM = "URI: ";
const char S_method[]             PROGMEM = "\nМетод: ";
const char S_args[]               PROGMEM = "\nАргументи: ";
const char S_parampre[]           PROGMEM = "param_";

// debug strings
const char D_HR[]                 PROGMEM = "--------------------";

#ifdef ESP8266
    const char S_ssidpre[]        PROGMEM = "Klimat";
#elif defined(ESP32)
    const char S_ssidpre[]        PROGMEM = "Klimat32";
#else
    const char S_ssidpre[]        PROGMEM = "Klimat";
#endif

#endif

#endif
