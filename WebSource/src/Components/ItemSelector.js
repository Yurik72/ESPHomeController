import React from "react";
import Button from "./Button"
import Popup from "reactjs-popup";
import { Card, Row, Col } from "./Card"
const getDefaultProps = () => {
    return {
        label: "",
        message: "",
        onSelect: (ev) => { },
        items:[]
    };
   
};
class ItemSelector extends React.Component {

    constructor(props) {
        super(props);


        //this.state = { isOn: false, isLdr: false, time: 0, bg: 150, wxspeed: 1000, color: 0, wxmode: 0 };
        this.getComponentprops = this.getComponentprops.bind(this);
        this.cs = {};
    };
    getComponentprops() {

        return { ...getDefaultProps(), ...this.props };
    }
    handleselectchange(ev, close) {
        const { onSelect } = this.props;
        if (onSelect)
            onSelect(ev.target.value);
        close();
    }
    render() {
       
        const { label, message, onSelect, items, valuekey, textkey, showcurrent, currentval } = this.getComponentprops();
       
        var itemsex = items;
        if ((itemsex || itemsex.length === 0) && (this.state && this.state.items && this.state.items.length !== 0))
            itemsex = this.state.items;
        var currentitem;
        
        if (showcurrent && itemsex) {
           
            var found = itemsex.find((item) => item[valuekey] === currentval);
           
            if (found)
                currentitem = found[textkey];
        }
       
        return (

            <Popup modal trigger={<Button className="left btn-small" label={label + (currentitem ? currentitem:"")} />} position="right center">
                {close => (
                    <div>
                        <Row>
                            <h5>
                                <span>{message}</span>
                             <a className="close" onClick={close}>
                                
                                </a>
                            </h5>
                        </Row>
                        <Row>

                            <select
                                onChange={(ev) => this.handleselectchange(ev, close)}
                                defaultValue="" required style={{ display: 'block' }}>
                                <option value="" disabled>{message}</option>
                                {itemsex.map((it, idx) =>
                                    <option key={idx} value={valuekey ? it[valuekey] : it} >{textkey?it[textkey]:it}</option>
                                )}
                                

                            </select>
                        </Row>

                    </div>
                )}
                </Popup>

        );
    }
}
export default ItemSelector;