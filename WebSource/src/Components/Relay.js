import React from "react";
import Checkbox from "./Checkbox";
import { getBaseuri, doFetch } from "./utils"

class Relay extends React.Component {
    constructor(props) {
        super(props);
        console.log(props);
        const { compprops } = props;
        this.state = { isOn: false };
        this.toggleCheckbox = ch => {
            console.debug("relay");
            console.debug(ch.state);
            let newstate = { isOn: ch.state.isChecked };
            this.setState(newstate);
            this.SendStateUpdate(newstate);
            
        };
        this.API = getBaseuri() + "/" + compprops.name;
       
    }
    componentDidMount() {
        //const { compprops } = this.props;
        //this.loadtest();
        //return;
        //alert(API);
        doFetch(this.API + "/get_state", (data) => { this.setState(data) });
        //fetch(this.API + "/get_state")
        //    .then(response => response.json())
        //    .then(data => { this.setState(data) });
            
        //alert(JSON.stringify(compprops));
        //this.setState({ services: compprops });
    }
    loadtest() {

        for (var i = 0; i < 20; i++) {
            doFetch(this.API + "/get_state", (data) => { this.setState(data) });
        }
    }
    SendStateUpdate(newstate) {

        return fetch(this.API + "/set_state", {
            method: 'post',
            mode: 'no-cors',
            body: JSON.stringify(newstate),
            headers: {
                'Content-Type': 'application/json'
            }
        }).then(res => {
            
            return res;
            }).catch(err => err);
    }

    render() {
        //{JSON.stringify(this.props)} 
        //const { label } = 'switch'; 
        const { compprops } = this.props;
    
        return (

    <div>
           
                <h2>{compprops.name} </h2>
                <div className="row">
                    <div className="col s12 "> 
                        <Checkbox
                            isChecked={this.state.isOn}
                            label={compprops.name}
                            handleCheckboxChange={this.toggleCheckbox}
                            key={compprops.name}
                        />
                </div>  

        </div>
    </div>
     );
    }
}

export default Relay;