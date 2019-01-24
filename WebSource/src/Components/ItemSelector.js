import React from "react";
import Button from "./Button"
import Popup from "reactjs-popup";

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
        //const dfltclassNames = "btn waves-effect waves-light btn_mode_static";
        const { label, message, onSelect, items,valuekey,textkey } = this.getComponentprops();
        //const { classNames } = this.props;
        var itemsex = items;
        if ((itemsex || itemsex.length === 0) && (this.state && this.state.items && this.state.items.length !== 0))
            itemsex = this.state.items;
       
        return (

            <Popup trigger={<Button label={label} />} position="right center">
                {close => (
                    <div>
                        <div className="row">
                            {message}
                             <a className="close" onClick={close}>
                                
                            </a>
                        </div>
                        <div className="row">

                            <select
                                onChange={(ev) => this.handleselectchange(ev, close)}
                                defaultValue="" required style={{ display: 'block' }}>
                                <option value="" disabled>{message}</option>
                                {itemsex.map((it, idx) =>
                                    <option key={idx} value={valuekey ? it[valuekey] : it} >{textkey?it[textkey]:it}</option>
                                )}
                                

                            </select>
                        </div>

                    </div>
                )}
            </Popup>
        );
    }
}
export default ItemSelector;