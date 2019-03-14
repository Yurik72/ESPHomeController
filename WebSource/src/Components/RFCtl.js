import React from "react";
import { getBaseuri, doFetch } from "./utils"
import { Card, Row, Col } from "./Card"
import Button from "./Button"

class RFCtl extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        this.state = {
            data: [], state: {} };

        this.API = getBaseuri() + "/" + compprops.name;
        this.doSend = this.doSend.bind(this);
    }

    componentDidMount() {

        doFetch(this.API + "/get_state", (data) => { this.setState({ state: data }) });
        doFetch(this.API + "/get_data", (data) => { this.setState({ data: data }) });
    
    }
    doSend(rfitem) {
        const uri = "/send?name=" + rfitem.name;
        doFetch(this.API + uri, (data) => {
        });
    }

    render() {

        const { compprops } = this.props;
        

        return (

            <Card title={() => { return (<p>{compprops.name} </p>); }}>
                <Row>
                    <Col num={2} className="green">
                        
                    </Col>
                </Row>
                <Row>
                    <Col num={2}>
                        <label htmlFor="rfkey" className="input-label" >RF KEY</label>
                        <input name="rfkey" type="text" value={this.state.state.rftoken} 
                        />
                    </Col>
                    <Col num={2}>
                        <label htmlFor="rfprotocol" className="input-label" >Protocol</label>
                        <input name="rfprotocol" type="text" value={this.state.state.rfprotocol} 
                        />
                    </Col>
                    <Col num={2}>
                        <label htmlFor="rflen" className="input-label" >Length</label>
                        <input name="rflen" type="text" value={this.state.state.rfdatalen}
                        />
                    </Col>
                    <Col num={2}>
                        <label htmlFor="rfpulse" className="input-label" >Pulse</label>
                        <input name="rfpulse" type="text" value={this.state.state.rfdelay} 
                        />
                    </Col>
                </Row>
                <Card title={() => { return (<p>Data</p>); }}>
                    {
                        this.state.data.map((item, idx) => {
                            return (
                                <Row>
                                    <Col num={2}>
                                        <label htmlFor="rfkey" className="input-label" >name</label>
                                        <input name="rfkey" type="text" value={item.name} name="name"
                                        />
                                    </Col>
                                    <Col num={2}>
                                        <label htmlFor="rfkey" className="input-label" >RF KEY</label>
                                        <input name="rfkey" type="text" value={item.token} name="token"
                                        />
                                    </Col>
                                    <Col num={2}>
                                        <label htmlFor="rfkey" className="input-label" >Protocol</label>
                                        <input name="rfkey" type="text" value={item.protocol} name="token"
                                        />
                                    </Col>
                                    <Col num={2}>
                                        <label htmlFor="rfkey" className="input-label" >Length</label>
                                        <input name="rfkey" type="text" value={item.len} name="len"
                                        />
                                    </Col>
                                    <Col num={2}>
                                        <label htmlFor="rfkey" className="input-label" >Pulse</label>
                                        <input name="rfkey" type="text" value={item.pulse} name="pulse"
                                        />
                                    </Col>
                                    <Col num={2}>
                                        <Button label="Send" onClick={()=>this.doSend(item)}
                                        />
                                            
                                    </Col>
                                </Row>
                                  );
                         })
                     }
                </Card>
            </Card>

        );
    }
}


export default RFCtl;