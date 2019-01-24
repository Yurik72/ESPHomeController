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
        this.setState(newState,
            () => {
                if (handleTimeChange)
                    handleTimeChange(this);
            });
    }
    render() {
        const { label, timevalue } = this.props;
        const skey =  "tm_" + label ;
        return (

            <>
                <label htmlFor={skey}>{label}</label><br />
                <p className="range-field">
                    <input type="timepicker"
                        ref={el => this.input = el}
                        id={skey}
                        value={timevalue}
                        onChange={this.handleChange}
                    /></p>
            </>
        )
    }

}

export default TimePickCtl;