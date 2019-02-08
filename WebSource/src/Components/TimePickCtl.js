import React from "react";

class TimePickCtl extends React.Component {
    constructor(props) {
        super(props);
       
        this.state = {

            timevalue: 0
        };
        this.input = {};
        this.handleChange = this.handleChange.bind(this);
    }

    handleChange(e) {
        const { handleTimeChange } = this.props;
        let newState = { timevalue: e.target.value };
        console.log("Time picker handle change");
        console.log(newState);
        this.setState(newState,
            () => {
                if (handleTimeChange)
                    handleTimeChange(this);
            });
    }
    getIntTime() {
        return parseInt(this.state.timevalue.replace(/(:)/gm, ""));
    }
    render() {
        const { label } = this.props;
        var { timevalue } = this.props;
        console.log(timevalue);
        if (typeof timevalue == 'number') {
            var hour= ((timevalue - timevalue % 100) / 100);
            var min = timevalue % 100;
            if (hour < 10)
                hour = "0" + hour;
            if (min < 10)
                min = "0" + min;
            timevalue = hour + ":" + min;
            
        }
        console.log(timevalue);
        const skey =  "tm_" + label ;
        return (

            <>
                <label htmlFor={skey} className="input-label">{label}</label><br />
                <p className="range-field">
                    <input type="time" min="00:00" max="24:00" required style={{fontSize:"2rem"}}
                        ref={el => this.input = el}
                        id={skey}
                        name={skey}
                        value={timevalue}
                        onChange={this.handleChange}
                    /></p>
            </>
        )
    }

}

export default TimePickCtl;