import React from "react";
import { getBaseuri, doFetch } from "./utils"

class BME280Ctl extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        this.state = { time: 0 };

        this.API = getBaseuri() + "/" + compprops.name;
      
    }

    componentDidMount() {

        doFetch(this.API + "/get_state", (data) => { this.setState(data) });
    }


    render() {

        const { compprops } = this.props;
        
        return (

            <div>

                <h2>{compprops.name} </h2>
                <div className="row">
                    <div className="col s12 green">
                        <h3>Temperature </h3>
                        <h4>{this.state.temp} </h4>
                    </div>

                </div>
                <div className="row">
                    <div className="col s12 green">
                        <h3>Pressure </h3>
                        <h4>{this.state.pres} </h4>
                    </div>

                </div>
                <div className="row">
                    <div className="col s12 green">
                        <h3>Humidity </h3>
                        <h4>{this.state.hum} </h4>
                    </div>

                </div>
            </div>
        );
    }
}

export default BME280Ctl;