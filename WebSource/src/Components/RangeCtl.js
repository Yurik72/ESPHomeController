import React from "react";
//import { setTimeout } from "timers";

class RangeCtl extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
          
            rangevalue: 0
        };
         this.handleChange = this.handleChange.bind(this);
         
    }
    handleChange(e) {
        const { handleRangeChange } = this.props;
       
        let newState = { rangevalue: e.target.value };
        var self = this;

        this.setState(newState, () => {
            if (handleRangeChange)
                handleRangeChange(self);
        });
       
    }
    render() {
        const { label,rangevalue } = this.props;
        return (

            <>
                <label htmlFor="txt_delay">{label}</label><br />
                <p className="range-field">
                    <input type="range"
                        id={"rng_" + label} min="0" max="255"
                        value={rangevalue}
                        onChange={this.handleChange} 
                         /></p>
            </>
        )
    }

}

export default RangeCtl;