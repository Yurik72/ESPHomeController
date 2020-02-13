import React, { Component } from 'react';


var _selectIndex = 1;
class Checkbox extends Component {
    constructor(props) {
        super(props);
        const { isChecked } = this.props;
        this.cid = "ch-" + _selectIndex++;
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
        var cid = this.cid;
        return (

            <div className="center switch__ ios-toggles"> <span>{label}</span>   <span></span>
            
                
                    <input
                    id={cid}
                        type="checkbox"
                        class="ios-toggle checkboxgreen"       
                        checked={isChecked}
                        onChange={this.toggleCheckboxChange}
                    />
                <label for={cid} class="checkbox-label" data-off="off" data-on="on"/>
                   
                    
                

            </div>
        );
    }
}

export default Checkbox;