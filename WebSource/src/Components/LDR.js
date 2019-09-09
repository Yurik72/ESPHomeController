import React from "react";
import { getBaseuri, doFetch} from "./utils"
import { Card, Row, Col } from "./Card"

class LDR extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        this.state = { ldrValue: 0 };
       
        this.API = getBaseuri()+"/" + compprops.name;

    }
    
    componentDidMount() {

        doFetch(this.API+"/get_state", (data) => { this.setState(data) });



    }


    render() {

        const { compprops } = this.props;

        return (
            <Card title={() => { return (<p>{compprops.name} </p>); }}>
                <Row>
                    <Col num={12} className="green">
                        <h3>{this.state.ldrValue} </h3>
                        <h3>{this.state.cValue ? this.state.cValue : ""} </h3>
                        <h3>{this.state.csValue ? this.state.csValue : ""} </h3>
                    </Col>
                </Row>
            </Card>

        );
    }
}

export default LDR;