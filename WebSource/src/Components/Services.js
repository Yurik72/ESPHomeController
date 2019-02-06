import React from "react";
import Button from "./Button"
import { getHomeUrl, getConfigData, getBaseuri } from "./utils"
import { Card, Row, Col } from "./Card"
import Arrow from "./Arrow"

class Services extends React.Component {
    constructor(props) {
        super(props);
        
        //const { compprops } = props;
        this.state = {services:[]};

        this.handleChange = (idx) => event => {
            //alert(idx);
            let services = [...this.state.services]; //all new copy
            
            let item = { ...services[idx] }; //edited item

            item[event.target.name] = event.target.value;
            alert(JSON.stringify(item));
            services[idx] = item;

            this.setState({ services });
            
        }
        this.state.expstate = {};
    }
    componentDidMount() {
        const { compprops } = this.props;
        this.setState({ services: compprops });
    }

    renderAllProps(item, sidx) {
        //console.debug(getBaseuri());
        return (
            <>
                <h3>Services</h3>
            <table className="tableservice">
               
                {
                   
                    Object.keys(item).map((key, idx) => {
                       
                        return <tr><td className="whiteSpaceNoWrap">
                            <div className="input-field col s6 l2">
                                <label for={sidx + key}>{key}</label>
                                <input type="text" id={sidx + key} name={key} value={item[key]}
                                    onChange={this.handleChange(sidx)}
                                    style={{ marginLeft:"200px" }}
                               />
                            </div>
                        </td></tr>
                    })
                }
                </table> 
            </>   
             );
            
           };
          
    render() {

        return ( 

            <div> 
               
                {
                    this.state.services.map((item, sidx) => {
                        var skey = "s" + sidx;
                        var expstate = this.state.expstate;
                        var isshow = expstate[skey];
                        return(
                            <>
                                <Card  hidecontent={!isshow} title={() => {
                                    return (

                                        <>

                                            <Col num={4}>
                                                <h6>{ item.service }</h6>
                                                
                                            </Col>

                                            <Col num={1} className="btn-collapse">
                                                <Button className="btn-collapse" nostyle={true} onClick={() => { expstate[skey] = !isshow; this.setState({ expstate }) }}
                                                    label={isshow ? "Hide" : "Show"}>
                                                    <Arrow dir={isshow ? "up" : "down"} />
                                                </Button>

                                            </Col>
                                        </>

                                    );
                                }}>
                                    {this.renderAllProps(item, sidx)}
                                </Card>
  
                        </>
                    )}
                    )
                }
            </div>
        );
    }
}
export default Services;