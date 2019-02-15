import React from "react";

const getDefaultProps= ()=>
    {
    return {
        width: 330,
        height: 330,
        onSelect: (intcolor, hexcolor) => { }
    };
};
class ColorWeel extends React.Component {

    constructor(props) {
        super(props);

        this.getMousePos = this.getMousePos.bind(this);
        this.setupColorWill = this.setupColorWill.bind(this);
        this.doClick = this.doClick.bind(this);
        this.updateStatus = this.updateStatus.bind(this);
        this.getComponentprops = this.getComponentprops.bind(this);
        this.doTouch = this.doTouch.bind(this);
    }

    getComponentprops() {

        return { ...getDefaultProps(), ...this.props };
    }

    componentDidMount() {
        
        //this.canvas = document.getElementById("myCanvas");
        this.context = this.canvas.getContext('2d');

        this.setupColorWill();

    }
    render() {

        const { width,height } = this.getComponentprops();

        return (

            <canvas  ref={el => this.canvas = el}  width={width} height={height}
                styles={{ WebkitUserSelect: "none", MozUserSelect: "none" }}></canvas>

        );
    }
    setupColorWill() {
        var canvas = this.canvas
        // FIX: Cancel touch end event and handle click via touchstart
        // canvas.addEventListener("touchend", function(e) { e.preventDefault(); }, false);
        canvas.addEventListener("touchmove", this.doTouch, false);
        canvas.addEventListener("click", this.doClick, false);
        //canvas.addEventListener("mousemove", doClick, false);


        var context = this.context;
        var centerX = canvas.width / 2;
        var centerY = canvas.height / 2;
        var innerRadius = canvas.width / 4.5;
        var outerRadius = (canvas.width - 10) / 2

        //outer border
        context.beginPath();
        //outer circle
        context.arc(centerX, centerY, outerRadius, 0, 2 * Math.PI, false);
        //draw the outer border: (gets drawn around the circle!)
        context.lineWidth = 4;
        context.strokeStyle = '#000000';
        context.stroke();
        context.closePath();

        //fill with beautiful colors 
        //taken from here: http://stackoverflow.com/questions/18265804/building-a-color-wheel-in-html5
        for (var angle = 0; angle <= 360; angle += 1) {
            var startAngle = (angle - 2) * Math.PI / 180;
            var endAngle = angle * Math.PI / 180;
            context.beginPath();
            context.moveTo(centerX, centerY);
            context.arc(centerX, centerY, outerRadius, startAngle, endAngle, false);
            context.closePath();
            context.fillStyle = 'hsl(' + angle + ', 100%, 50%)';
            context.fill();
            context.closePath();
        }

        //inner border
        context.beginPath();
        //context.arc(centerX, centerY, radius, startAngle, endAngle, counterClockwise);
        context.arc(centerX, centerY, innerRadius, 0, 2 * Math.PI, false);
        //fill the center
        var my_gradient = context.createLinearGradient(0, 0, 170, 0);
        my_gradient.addColorStop(0, "black");
        my_gradient.addColorStop(1, "white");

        context.fillStyle = my_gradient;
        context.fillStyle = "white";
        context.fill();

        //draw the inner line
        context.lineWidth = 2;
        context.strokeStyle = '#000000';
        context.stroke();
        context.closePath();
    }
    doTouch(event) {
        //to not also fire on click
        event.preventDefault();
        var el = event.target;

        //touch position
        var pos = {
            x: Math.round(event.targetTouches[0].pageX - el.offsetLeft),
            y: Math.round(event.targetTouches[0].pageY - el.offsetTop)
        };
        //color
        var color = this.context.getImageData(pos.x, pos.y, 1, 1).data;

        this.updateStatus(pos, color);
    }

    doClick(event) {
        //click position   
        var pos = this.getMousePos(this.canvas, event);
        //color
       
        this.context = this.canvas.getContext('2d');
        var color = this.context.getImageData(pos.x, pos.y, 1, 1).data;
        console.log(color);
        //console.log("click", pos.x, pos.y, color);
        this.updateStatus(pos, color);

        //now do sth with the color rgbToHex(color);
        //don't do stuff when #000000 (outside circle and lines
    }
    getMousePos(canvas, evt) {
        var rect = this.canvas.getBoundingClientRect();
        return {
            x: evt.clientX - rect.left,
            y: evt.clientY - rect.top
        };
    }
    //comp to Hex
    componentToHex(c) {
        //var hex = c.toString(16);
        //return hex.length == 1 ? "0" + hex : hex;
        return ("0" + (Number(c).toString(16))).slice(-2).toUpperCase();
    }

    //rgb/rgba to Hex
    rgbToHex(rgb) {
        return this.componentToHex(rgb[0]) + this.componentToHex(rgb[1]) + this.componentToHex(rgb[2]);
    }
    intToHex(val) {
        let hexString = val.toString(16);
        if (hexString.length % 2) {
            hexString = '0' + hexString;
        }
        return hexString;
    }
    updateStatus(pos, color) {
        const { onSelect } = this.getComponentprops();
        
        var hexColor = this.rgbToHex(color);
        console.log(hexColor);
        var intcolor = parseInt("0x" + hexColor);
        //wsSetAll(hexColor);
        console.log(intcolor);

        hexColor = "#" + hexColor;
        onSelect(intcolor, hexColor);


    }
}
export default ColorWeel;