import React, { Component } from 'react';
import { Row,Col } from './Card';
import Select from 'react-select'

import { convertCronToString } from './utils'


class CronEdit extends Component {
    constructor(props) {
        super(props);
        const { isChecked, cronvalue } = this.props;
        if (!cronvalue)
            cronvalue = "* * * * * *";
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

        this.cron_seconds = "*";
        this.cron_minutes = "*";
        this.cron_hours = "*";
        this.cron_dom = "*";
        this.cron_months = "*";
        this.cron_dow = "*";

        this.cron_selhours = undefined;
        this.cron_selminutes = undefined;
        this.cron_seldoms = undefined;
        this.cron_selmonths = undefined;
        this.cron_seldows = undefined;
        this.state = { cron_res: cronvalue, cron_hum: convertCronToString(cronvalue) };

        this.importCronExpression(cronvalue);
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
        for (var i = 0; i < 24; i++) {
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
   importCronExpression(expression) {
    if (!expression.match(/^((\*(\/[1-9][0-9]?)?|([0-9]{1,2}(-[0-9]{1,2})?)(,[0-9]{1,2}(-[0-9]{1,2})?)*)( |$)){6}$/))
        return;
    var parts = expression.split(" ");

    var tmp;
    if (parts[1] != this.cron_minutes) {
        this.cron_minutes = parts[1];
        this.cron_selminutes = this.cronValueItemToList(true, 59, parts[1]);
        //cronHelperSelectList(cron_minutes_id, (true, 59, parts[0]));
    }
       if (parts[2] != this.cron_hours) {
           this.cron_hours = parts[2];
           this.cron_selhours = this.cronValueItemToList(true, 23, parts[2]);

        //cronHelperSelectList(cron_hours_id, cronValueItemToList(true, 23, parts[1]));
    }
       if (parts[3] != this.cron_dom) {
           this.cron_dom = parts[3];
           this.cron_seldom = this.cronValueItemToList(true, 31, parts[3]);
        //cronHelperSelectList(cron_dom_id, cronValueItemToList(false, 31, parts[2]));
    }
       if (parts[4] != this.cron_months) {
           this.cron_months = parts[4];
           this.cron_selmonths = this.cronValueItemToList(true, 12, parts[4]);
        //cronHelperSelectList(cron_months_id, cronValueItemToList(false, 12, parts[3]));
    }
       if (parts[5] != this.cron_dow) {
           this.cron_dow = parts[5];
           this.cron_seldows = this.cronValueItemToList(true, 6, parts[5]);
        
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
    } 


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
     return list.map(function (el,idx) { return {value:el,label:el}});
    }  

    updateField(type,lst) {
        var zeroAllowed = true;
        var selvals=[];
        if (Array.isArray(lst))
          selvals=lst.map(function (element) { return element.value; })
    switch (type) {
        case "hours":
            this.cron_hours = this.cronCalculate(true, 23, selvals);
            this.cron_selhours = lst;
            if (Array.isArray(this.cron_selhours)
                && this.cron_selhours.length > 0
                && !Array.isArray(this.cron_minutes)) {
                this.cron_selminutes = this.minutes[0];
                this.cron_minutes = "0";
            }
            break;
        case "minutes":
            this.cron_minutes = this.cronCalculate(true, 59, selvals);
            this.cron_selminutes = lst;
            break;
        case "dom":
            this.cron_dom = this.cronCalculate(false, 31, selvals);
            this.cron_seldoms = lst;
            break;
        case "months":
            this.cron_months = this.cronCalculate(false, 12, selvals);
            this.cron_selmonths = lst;
            break;
        case "dow":
            this.cron_dow = this.cronCalculate(true, 6, selvals);
            this.cron_seldows = lst;
            break;
    }
        this.on_Change(this.getCronExpression());
        this.setState({ cron_res: this.getCronExpression() });
        this.setState({ cron_hum: convertCronToString(this.getCronExpression() )});
        //console.log(this.getCronExpression());
    } 
    getCronExpression() {
        return this.cron_seconds + " " + this.cron_minutes + " " + this.cron_hours + " " + this.cron_dom + " " +
            this.cron_months + " " + this.cron_dow;
    } 

    handlehoursChange = (selectedOption) => {
        console.log(selectedOption);
        this.updateField('hours', selectedOption.map( function (element) { return element.value; }));
    }
    handlecronChange = (selectedOption,type) => {
        console.log(selectedOption);
        this.updateField(type, selectedOption);
    }
    render() {
        console.log(this.cron_selhours);
        return (
           
           <>   
            
            <Row>

                <Col num={3}>
                <label>Hours</label>
                        <Select options={this.hours}
                            value={this.cron_selhours}
                    isMulti
                    name="hours"
                    onChange={s => { this.handlecronChange(s,'hours');}}
                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={3}>
                <label>Minutes</label>
                <Select options={this.minutes}
                    isMulti
                    name="minutes"
                    value={this.cron_selminutes}
                    onChange={s => { this.handlecronChange(s, 'minutes'); }}
                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={3}>
                <label>Days</label>
                <Select options={this.days}
                    isMulti
                            name="days"
                            value={this.cron_seldoms}
                    onChange={s => { this.handlecronChange(s, 'dom'); }}
                    className="basic-multi-select"
                    classNamePrefix="select"
                    />
                </Col>
                <Col num={3}>
                <label>Months</label>
                <Select options={this.months}
                            isMulti
                            name="months"
                            value={this.cron_selmonths}
                            onChange={s => { this.handlecronChange(s, 'months'); }}
                            className="basic-multi-select"
                            classNamePrefix="select"
                    />
                </Col>
                <Col num={3}>
                <label>DayOfWeek</label>
                <Select options={this.dayofweek}
                            isMulti
                            name="dayofweek"
                            value={this.cron_seldows}
                            onChange={s => { this.handlecronChange(s, 'dow'); }}
                            className="basic-multi-select"
                            classNamePrefix="select"
                    />
                </Col>
            </Row>
            <Row>
                <Col num={12}>
                        <h5><input value={this.state.cron_res}
                            onChange={e =>

                            {
                                var val = e.target.value;

                                this.importCronExpression(val);
                                this.setState({ cron_res: this.getCronExpression() });
                                this.setState({ cron_hum: convertCronToString(this.getCronExpression()) });
                            }}
                        /></h5>
                   
                </Col>
            </Row>
            <Row>
                <Col num={12}>
                    <h5>{this.state.cron_hum} </h5>
                   
                </Col>
            </Row>
        </>
        );
    }
}

export default CronEdit;