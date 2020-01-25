import React, { Component } from 'react';
import { Row,Col } from './Card';
import Select from 'react-select'


class CronEdit extends Component {
    constructor(props) {
        super(props);
        const { isChecked } = this.props;
        this.days = [];
        this.hours = [];
        this.minutes = [];
        this.months = [];
        this.dayofweek = [];
        this.initdays();
        this.inithours();
        this.initminutes();
        this.initmonth();
        this.initdayofweek();
        this.cron_minutes = "*";
        this.cron_hours = "*";
        this.cron_dom = "*";
        this.cron_months = "*";
        this.cron_dow = "*";
       
        const { onCronChange } = this.props;
        this.on_Change = onCronChange;
        if (!onCronChange)
            this.on_Change= (value) => { };
    }
    initdayofweek() {
        for (var i = 0; i <=6; i++) {
            this.dayofweek.push({ value: i, label: i });
        }
    }
    initdays() {
        for (var i = 0; i < 31; i++) {
            this.days.push({ value: i, label: i });
        }
    }
    inithours() {
        for (var i = 0; i <= 24; i++) {
            this.hours.push({ value: i, label: i });
        }
    }
    initminutes() {
        for (var i = 0; i <= 30; i++) {
            this.minutes.push({ value: i, label: i });
        }
    }
    initmonth() {
        for (var i = 0; i <= 12; i++) {
            this.months.push({ value: i, label: i });
        }
    }
   
 cronCalculate(zeroAllowed, max, values) {
    if (zeroAllowed == false)
        values.push(0);
    if (values.length > max || values.length == 0)
        return "*";
    values.sort(function (a, b) { return a - b });
    // divider ("*/n")
    out:
    for (var d = 2; d <= Math.ceil(max / 2); d++) {
        var tmp = values.slice();
        for (var x = 0; x * d <= max; x++) {
            if (tmp.indexOf(x * d) == -1)
                continue out;
            else
                tmp.splice(tmp.indexOf(x * d), 1);
        }
        if (tmp.length == 0)
            return "*/" + d;
    }
    // if not allowed, remove 0
    if (zeroAllowed == false)
        values.splice(values.indexOf(0), 1);
    // ranges ("2,8,20,25-35")
    var output = values[0] + "";
    var range = false;
    for (var i = 1; i < values.length; i++) {
        if (values[i - 1] + 1 == values[i]) {
            range = true;
        } else {
            if (range)
                output = output + "-" + values[i - 1];
            range = false;
            output = output + "," + values[i];
        }
    }
    if (range)
        output = output + "-" + values[values.length - 1];
    return output;
} // }}}
/** Convert a cron-expression (one item) to a list of values {{{
 * @param bool zeroAllowed  weather the number zero is allowed (true) or not
 *                          (false)
 * @param  int              max the maximum value (eg. 59 for minutes)
 * @param  string           the cron expression (eg. "*")
 * @return int[]
 */
 cronValueItemToList(allowZero, maxValue, value) {
    var list = [];
    if (value == "*") {
        for (var i = allowZero ? 0 : 1; i <= maxValue; i++) {
            list.push(i);
        }
    } else if (value.match(/^\*\/[1-9][0-9]?$/)) {
        var c = parseInt(value.match(/^\*\/([1-9][0-9]?)$/)[1]);
        for (var i = allowZero ? 0 : 1; i <= maxValue; i++) {
            if (i % c == 0)
                list.push(i);
        }
    } else if (value.match(/^([0-9]+|[0-9]+-[0-9]+)(,[0-9]+|,[0-9]+-[0-9]+)*$/)) {
        var a = value.split(",");
        for (var i = 0; i < a.length; i++) {
            var e = a[i].split("-");
            if (e.length == 2) {
                for (var j = parseInt(e[0]); j <= parseInt(e[1]); j++)
                    list.push(j);
            } else {
                list.push(parseInt(e[0]));
            }
        }
    } else {
        return [];
    }
    return list;
    }  

    updateField(type,lst) {
   var  zeroAllowed = true;
    switch (type) {
        case "hours":
            this.cron_hours = this.cronCalculate(true, 23, lst);
            
            break;
        case "minutes":
            this.cron_minutes = this.cronCalculate(true, 59, lst);
            break;
        case "dom":
            this.cron_dom = this.cronCalculate(false, 31, lst);
            break;
        case "months":
            this.cron_months = this.cronCalculate(false, 12, lst);
            break;
        case "dow":
            this.cron_dow = this.cronCalculate(true, 6, lst);
            break;
    }
        this.on_Change(this.getCronExpression());
        console.log(this.getCronExpression());
    } 
    getCronExpression() {
        return this.cron_minutes + " " + this.cron_hours + " " + this.cron_dom + " " +
            this.cron_months + " " + this.cron_dow;
    } 

    handlehoursChange = (selectedOption) => {
        console.log(selectedOption);
        this.updateField('hours', selectedOption.map( function (element) { return element.value; }));
     }
    render() {
        
        return (
           
              
            
            <Row>
                <Col num={4}>
                <label>Hours</label>
                <Select options={this.hours}
                    isMulti
                    name="hours"
                    onChange={this.handlehoursChange}
                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={4}>
                <label>Minutes</label>
                <Select options={this.minutes}
                    isMulti
                    name="minutes"

                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={4}>
                <label>Days</label>
                <Select options={this.days}
                    isMulti
                    name="days"
                   
                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={4}>
                <label>Months</label>
                <Select options={this.months}
                    isMulti
                    name="months"

                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={4}>
                <label>DayOfWeek</label>
                <Select options={this.dayofweek}
                    isMulti
                    name="dayofweek"
                   
                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
         </Row>
        );
    }
}

export default CronEdit;