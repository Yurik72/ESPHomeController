import React from "react";

class ColorStatus extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            style: { backgroundColor: "#000000" }
        };
        this.setBkColor = this.setBkColor.bind(this);
        const { color } = props;
        //this.setBkColor(color);
        
    }
    render() {
       // console.log("render color");
        let style = { ...this.state.style };
        const { color } = this.props;

        var rendercolor = color ? parseInt(color) : this.state.style.backgroundColor;
        
        //console.log(rendercolor);
        if (rendercolor === parseInt(rendercolor, 10)) {//is int
            rendercolor = this.intToHex(rendercolor);
        }
       // console.log(rendercolor);
        rendercolor = rendercolor.toString();
        
        if (rendercolor && !rendercolor.startsWith("#"))
            rendercolor = "#" + rendercolor;
            
        style.backgroundColor = rendercolor;
        //console.log(rendercolor);
     return (
            
     <div
         style={style}
                ref={a => this._acc = a}>
                <div id="status_pos">color value</div>
             <div id="status_color">{rendercolor}</div>
             <div id="statusint_color">{color}</div>
                {this.props.children}
            </div>
        )
    }
    componentToHex(c) {
        //var hex = c.toString(16);
        //return hex.length == 1 ? "0" + hex : hex;
        return ("0" + (Number(c).toString(16))).slice(-2).toUpperCase();
    }

    //rgb/rgba to Hex
    rgbToHex(rgb) {
        return this.componentToHex(rgb[0]) + this.componentToHex(rgb[1]) + this.componentToHex(rgb[2]);
    }
    setBkColor(col) {

        this.setState({ style: { backgroundColor: col } });
    }
    intToHex(val) {
        let hexString = val.toString(16);
        if (hexString.length % 2) {
            hexString = '0' + hexString;
        }
        return hexString;
    }
}
                
export default ColorStatus;