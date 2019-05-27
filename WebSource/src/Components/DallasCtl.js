import React from "react";
import { getBaseuri, doFetch } from "./utils"

class DallasCtl extends React.Component {
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

            </div>
        );
    }
}

export default DallasCtl;