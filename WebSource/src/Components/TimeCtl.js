import React from "react";
import { getBaseuri, doFetch } from "./utils"
import Card from "./Card"

class TimeCtl extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        this.state = { time: 0 };

        this.API = getBaseuri() + "/" + compprops.name;
        this.time_tToHHMM = this.time_tToHHMM.bind(this);
    }
   time_tToHHMM(tt) {
        var dt = new Date(1000 * (tt - 7200));
        var res = dt.getHours() < 10 ? "0" + dt.getHours().toString() : dt.getHours().toString();
        res += ":";
        res += dt.getMinutes() < 10 ? "0" + dt.getMinutes().toString() : dt.getMinutes().toString();
        return res;
    }
    componentDidMount() {

        doFetch(this.API + "/get_state", (data) => { this.setState(data) });
    }


    render() {

        const { compprops } = this.props;
        const tm = this.time_tToHHMM(this.state.time);
        
        return (

            <Card title={() => { return (<p>{compprops.name} </p>); }}>
                     <div className="row">
                        <div className="col s12 green">
                            <h3>{tm} </h3>
                        </div>
                    </div>
            </Card>
        );
    }
}

export default TimeCtl;