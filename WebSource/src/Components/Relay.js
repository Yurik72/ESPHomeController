import React from "react";
import Checkbox from "./Checkbox";
import { getBaseuri, doFetch } from "./utils"
import { Card, Row ,Col} from "./Card"

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

        doFetch(this.API + "/get_state", (data) => { this.setState(data) });

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

        const { compprops } = this.props;
    
        return (
            <Card title={() => { return (<h3>{compprops.name} </h3>); }}>
                <Row>
                    <Col num={12}>
                        <Checkbox
                            isChecked={this.state.isOn}
                            label={compprops.name}
                            handleCheckboxChange={this.toggleCheckbox}
                            key={compprops.name}
                        />
                    </Col>
                </Row>
            </Card>


     );
    }
}

export default Relay;