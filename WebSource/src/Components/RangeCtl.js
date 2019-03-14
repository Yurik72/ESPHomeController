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
        const { label, rangevalue,minval,maxval } = this.props;
        let min = minval?minval:0;
        let max = maxval ?maxval:255;
        return (

            <>
                <label htmlFor={"rng_" + label} className="input-label">{label + " " + rangevalue}</label><br />
                <p className="range-field">
                    <input type="range"
                        id={"rng_" + label}
                        name={"rng_" + label}
                        min={min} max={max}
                        value={rangevalue}
                        onChange={this.handleChange} 
                         /></p>
            </>
        )
    }

}

export default RangeCtl;