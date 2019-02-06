import React, { Component } from 'react';

class Checkbox extends Component {
    constructor(props) {
        super(props);
        const { isChecked } = this.props;
        
        this.state = {
            isChecked: isChecked
        }
    }
    

    toggleCheckboxChange = () => {
        const { handleCheckboxChange } = this.props;
        console.debug("toggleCheckboxChange");
        console.debug(this.state);

        this.setState({ ...this.state, ...{ isChecked: !this.state.isChecked } }
            , function () { handleCheckboxChange(this) }
        );

        
    }
    componentWillReceiveProps(nextProps) {

        if (nextProps.isChecked !== this.props.isChecked) {
            const { isChecked} = nextProps;

            this.setState({ isChecked: isChecked });
        }
    }
    render() {
        const { label } = this.props;
        const { isChecked } = this.props;
       
        return (
           
                <div className="center switch">{label}<br />
                <label>Off
                    <input
                        type="checkbox"
                        
                        checked={isChecked}
                        onChange={this.toggleCheckboxChange}
                    />
                    <span className="lever"></span>On 
                    
                    </label>

            </div>
        );
    }
}

export default Checkbox;