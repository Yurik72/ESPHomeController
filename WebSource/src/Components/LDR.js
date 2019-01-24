import React from "react";
import { getBaseuri, doFetch} from "./utils"

class LDR extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        this.state = { ldrValue: 0 };
       
        this.API = getBaseuri()+"/" + compprops.name;

    }
    
    componentDidMount() {
        //const { compprops } = this.props;

       // console.log(this.API);
        doFetch(this.API+"/get_state", (data) => { this.setState(data) });
       /*
        fetch(this.API + "/get_state")
            .then(response => {
                console.log(response);
               return response.json()
            }
            )
            .then(data => { this.setState(data) })
            .catch(function (error) {
                console.log("123211212");
                console.log(error);
             });
       
       */


    }


    render() {

        const { compprops } = this.props;

        return (

            <div>

                <h2>{compprops.name} </h2>
                <div className="row">
                    <div className="col s12 green">
                        <h3>{this.state.ldrValue} </h3>
                    </div>

                </div>
            </div>
        );
    }
}

export default LDR;