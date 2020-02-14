

var HomeURL = 'http://192.168.88.17'
//var HomeURL = 'http://192.168.0.213'

var i = 0;
export function getConfigData() {

}

export function getHomeUrl() {
    return HomeURL;
}

export function getServices() {
    return fetch(getBaseuri() + "services.json");
}
export function getBaseuri() {
    var path;
   // console.log(process.env.NODE_ENV);
    if (typeof window !== 'undefined') {
        var path = window.location.protocol + '//' + window.location.host ; // (or whatever)
    } else {
        path = HomeURL;
    } 
    //return path;
    if (process.env.NODE_ENV === 'development')
        path = HomeURL;
    return path;
}
var entrance = 0;
const timeout = 300;
export function doFetch(apiurl, callback) {
    
   // console.log("entrance count" + entrance);
    setTimeout(() => internaldoFetch(apiurl, callback, 3, () => {
        entrance--;
   //     console.log("entrance leave:" + entrance);
    }), (entrance * timeout));
    entrance++;
}

function internaldoFetch(apiurl, callback, iter,leave) {
   
    if (!iter || iter <= 0) {
        console.log("leaved by limit range" + apiurl+"->" + entrance);
        leave();
        return;
    }
    //console.log("execution " + apiurl );
    fetch(apiurl)
        .then(response => {
            leave();
           // console.log("success leave" + apiurl + "->" + entrance)
            if (response.status === 200) {
                //console.log(response);
                return response.json()
            }
            else {
               
                console.log("on fetch  " + apiurl)
            }
        }
        )
        .then(data => callback(data))
        .catch(function (error) {
            console.log("ops ! fetch is fail on iteration " + iter)
            console.log(error);
            setTimeout(()=> { internaldoFetch(apiurl, callback, --iter, leave) }, timeout);
           // timeout += 100;
        });
}
export function constrain (aNumber, aMin, aMax) {
    return aNumber > aMax ? aMax : aNumber < aMin ? aMin : aNumber;
};
export function inrange(aNumber, aMin, aMax) {
    return aNumber>=aMin && aNumber <= aMax; 
};
export function map(value, istart, istop, ostart, ostop) {
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
};
export function mapInt(value, istart, istop, ostart, ostop) {
    return parseInt(map(value, istart, istop, ostart, ostop),10);
};

export function string_chop(str, size) {
    if (str == null) return [];
    str = String(str);
    size = ~~size;
    return size > 0 ? str.match(new RegExp('.{1,' + size + '}', 'g')) : [str];
}
export function encode_chops(arr) {
    var total = 0;
    for (var i = 0; i < arr.length; i++) {
        arr[i] = encodeURI(arr[i]);
        total += arr[i].length;
    }
    return total;
}

export function convertCronToString(cronExpression) {
    if (typeof (cronExpression) != 'string' || !cronExpression.match(/^((\*(\/[1-9][0-9]?)?|([0-9]{1,2}(-[0-9]{1,2})?)(,[0-9]{1,2}(-[0-9]{1,2})?)*)( |$)){6}$/))
        return "invalid expression";
    var cron = cronExpression.split(" ");
    var minutes = cron[1];
    var hours = cron[2];
    var dayOfMonth = cron[3];
    var month = cron[4];
    var dayOfWeek = cron[5];

    var cronToString = "Runs at ";

    // Formatting time if composed of zeros
    if (minutes === "0") minutes = "00";
    if (hours === "0") hours = "00";
    // If it's not past noon add a zero before the hour to make it look like "04h00" instead of "4h00"
    else if (hours.length === 1 && hours !== "*") {
        hours = "0" + hours;
    }
    // Our activities do not allow launching pipelines every minute. It won't be processed.
    if (minutes === "*") {
        //cronToString =
        //    "Unreadable cron format. Cron will be displayed in its raw form: " +
        //    cronExpression;
    }
    var monthstr = "";
    if (month === "*") {
        monthstr = " of every month";
    }
    else {
        monthstr =" of " + month + " month ";
    }

    cronToString = cronToString + hours + "h" + minutes + " ";

    if (dayOfWeek === "0,6") dayOfWeek = "on weekends";
    else if (dayOfWeek === "1-5") dayOfWeek = "on weekdays";
    else if (dayOfWeek.length === 1) {
        if (dayOfWeek === "*" && dayOfMonth === "*") dayOfWeek = "every day ";
        else if (dayOfWeek === "*" && dayOfMonth !== "*") {
            cronToString = cronToString + "on the " + dayOfMonth;
            if (
                dayOfMonth === "1" ||
                dayOfMonth === "21" ||
                dayOfMonth === "31"
            ) {
                cronToString = cronToString + "st ";
            } else if (dayOfMonth === "2" || dayOfMonth === "22") {
                cronToString = cronToString + "nd ";
            } else if (dayOfMonth === "3" || dayOfMonth === "23") {
                cronToString = cronToString + "rd ";
            } else {
                cronToString = cronToString + "th ";
            }
            cronToString = cronToString + "day " + monthstr;//of every month";
            return cronToString;
        } else if (dayOfWeek !== "*" && dayOfMonth === "*") {
            switch (parseInt(dayOfWeek)) {
                case 0:
                    dayOfWeek = "on Sundays";
                    break;
                case 1:
                    dayOfWeek = "on Mondays";
                    break;
                case 2:
                    dayOfWeek = "on Tuesdays";
                    break;
                case 3:
                    dayOfWeek = "on Wednesdays";
                    break;
                case 4:
                    dayOfWeek = "on Thursdays";
                    break;
                case 5:
                    dayOfWeek = "on Fridays";
                    break;
                case 6:
                    dayOfWeek = "on Saturdays";
                    break;
                default:
                    cronToString =
                        "Unreadable cron format. Cron will be displayed in its raw form: " +
                        cronExpression;
                    return cronToString;
            }
        }
        cronToString = cronToString + dayOfWeek + " " + monthstr;
    }

    return cronToString;
}