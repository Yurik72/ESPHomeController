

var HomeURL = 'http://192.168.0.2'
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
const timeout = 100;
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