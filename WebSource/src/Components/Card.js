import React from "react";

class Card extends React.Component {
   


    render() {
       // console.log(this.props.children);
        return (

            <div className="card" >
                <div className="row blue-grey lighten-3 valign-wrapper cardtitle" >
                    {this.props.title ? this.props.title():""}
                </div>
                {!this.props.hidecontent &&
                    <div className="card-content">
                        {this.props.children}
                    </div>
                }
            </div>
        );
    }
}
const Row = (props) => {

    const { style } = props ? props : { style: {}};

    return (
        <div className={"row " + (props.className ? props.className : "")} style={style}>
            {props.children}
        </div>
    );
}
const Col = (props) => {
    const getclass = (num) => { return num ? "s" + num + " " : " "; };
    const basestyle = {};
    const { style } = props;
    return (
        <div className={"col " + getclass(props.num) + (props.className ? props.className : "")} style={{ ...basestyle, ...style }}>
            {props.children}
        </div>
    );
}
export default Card;
export { Card, Row, Col };

