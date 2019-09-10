import React from "react";
import { getBaseuri, doFetch} from "./utils"
import { Card, Row, Col } from "./Card"

class ThingSpeak extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        this.state = { data: [] };
       
        this.API = getBaseuri()+"/" + compprops.name;

    }
    
    componentDidMount() {

        doFetch(this.API+"/get_state", (data) => { this.setState(data) });



    }


    render() {

        const { compprops } = this.props;

        return (
            <Card title={() => { return (<p>{compprops.name} </p>); }}>
                {this.state.data.map((it, idx) =>
                    <Row>
                        <Col num={4} className="gray">
                            Channel {idx}
                        </Col>
                        <Col num={4} className="green">
                            {it}
                        </Col>
                    </Row>
                    
                )}


            </Card>

        );
    }
}

export default ThingSpeak;